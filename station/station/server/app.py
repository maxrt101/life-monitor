from flask import Flask, render_template, request, redirect, url_for, session, flash, jsonify, g
from werkzeug.security import generate_password_hash, check_password_hash
from station import db
from station.radio import StatusFlags, AlertTrigger
import datetime


app = Flask('LifeMonitorStation', template_folder='station/server/templates')
app.config['SECRET_KEY'] = 'your-very-secret-key-for-sessions'


def get_device_status(device):
    """
    Analyzes a device's latest status and alerts to determine its status.
    """
    try:
        # 1. Check for recent critical alerts
        recent_alert = (db.Alert.select()  # <-- Added db. prefix
                        .where(db.Alert.device == device)
                        .order_by(db.Alert.timestamp.desc())
                        .get_or_none())

        if recent_alert:
            # Check if alert is very recent (e.g., last 5 mins)
            time_diff = datetime.datetime.now() - recent_alert.timestamp
            if time_diff.total_seconds() < 300:
                # Map trigger codes to messages
                trigger_msg = f"TRIGGER={recent_alert.trigger}"
                if recent_alert.trigger == AlertTrigger.PULSE_THRESHOLD.value: trigger_msg = "Pulse Alert"
                if recent_alert.trigger == AlertTrigger.SUDDEN_MOVEMENT.value: trigger_msg = "Sudden Movement Detected"
                return "CRITICAL", f"Alert: {trigger_msg}"

        # 2. Check latest status report
        latest_status = (db.Status.select()  # <-- Added db. prefix
                         .where(db.Status.device == device)
                         .order_by(db.Status.timestamp.desc())
                         .get_or_none())

        if not latest_status:
            return "WARNING", "No Status Data"

        # 3. Check for offline (no status in 2 minutes)
        time_diff = datetime.datetime.now() - latest_status.timestamp
        if time_diff.total_seconds() > 300:  # Your 5-minute threshold
            return "CRITICAL", "Offline (No Signal)"

        # 4. Check status flags
        if latest_status.flags & StatusFlags.PULSE_SENSOR_FAILURE:  # <-- Added StatusFlags. prefix
            return "WARNING", f"Pulse Sensor Failure ({latest_status.bpm} BPM)"
        if latest_status.flags & StatusFlags.ACCEL_SENSOR_FAILURE:  # <-- Added StatusFlags. prefix
            return "WARNING", f"Accel Sensor Failure ({latest_status.bpm} BPM)"
        if latest_status.flags & StatusFlags.GPS_FAILURE:  # <-- Added StatusFlags. prefix
            return "WARNING", f"GPS Failure ({latest_status.bpm} BPM)"

        # 5. Check pulse
        if latest_status.bpm == 0:
            return "CRITICAL", "No Pulse Detected"
        if latest_status.bpm < 40 or latest_status.bpm > 180:
            return "WARNING", f"Abnormal Pulse ({latest_status.bpm} BPM)"

        # If all checks pass:
        return "OK", f"Nominal ({latest_status.bpm} BPM)"

    except Exception as e:
        return "WARNING", f"Error ({e})"


def convert_gps_to_signed(value, direction):
    """Converts (12.34, 'S') to -12.34"""
    if direction in ['S', 'W']:
        return -value
    return value


# --- Authentication Routes ---

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        try:
            user = db.User.get(db.User.username == username)
            if check_password_hash(user.password_hash, password):
                session['user_id'] = user.id
                session['username'] = user.username
                return redirect(url_for('dashboard'))
            else:
                flash('Invalid username or password')
        except Exception:  # Changed from User.DoesNotExist to catch all
            flash('Invalid username or password')
    return render_template('login.html')


@app.route('/logout')
def logout():
    session.pop('user_id', None)
    session.pop('username', None)
    return redirect(url_for('login'))


# --- Protected App Routes ---

@app.before_request
def before_request_handler():
    """Runs before every request."""
    # Ensure user is logged in for all routes except login/static
    if 'user_id' not in session and request.endpoint not in ['login', 'static']:
        return redirect(url_for('login'))

    # Make device list available to all templates (for the debug menu)
    if 'user_id' in session:
        g.all_devices = db.Device.select()
    else:
        g.all_devices = []


@app.route('/')
@app.route('/dashboard')
def dashboard():
    devices = []
    for device in g.all_devices:  # Use the global list
        status, message = get_device_status(device)
        devices.append({
            'mac': device.mac,
            'name': device.name,
            'version': device.version,
            'status': status,
            'message': message
        })
    return render_template('dashboard.html', devices=devices)


@app.route('/device/<int:device_mac>')
def device_detail(device_mac):
    try:
        device = db.Device.get(db.Device.mac == device_mac)  # <-- Use db. prefix
        status, message = get_device_status(device)

        # Get recent logs
        status_logs_query = (db.Status.select()  # <-- Use db. prefix
                             .where(db.Status.device == device)
                             .order_by(db.Status.timestamp.desc())
                             .limit(50))

        alert_logs_query = (db.Alert.select()  # <-- Use db. prefix
                            .where(db.Alert.device == device)
                            .order_by(db.Alert.timestamp.desc())
                            .limit(50))

        alert_logs_query = (db.Alert.select()
                            .where(db.Alert.device == device)
                            .order_by(db.Alert.timestamp.desc())
                            .limit(50))

        # Process logs to include descriptive text
        status_logs = []
        for log in status_logs_query:
            flags_list = []
            if log.flags & StatusFlags.PULSE_SENSOR_FAILURE: flags_list.append(
                "PULSE FAIL")  # <-- Use StatusFlags. prefix
            if log.flags & StatusFlags.ACCEL_SENSOR_FAILURE: flags_list.append(
                "ACCEL FAIL")  # <-- Use StatusFlags. prefix
            if log.flags & StatusFlags.GPS_FAILURE: flags_list.append("GPS FAIL")  # <-- Use StatusFlags. prefix
            status_logs.append({
                "log": log,
                "flags_text": ", ".join(flags_list) or "OK"
            })

        alert_logs = []
        for log in alert_logs_query:
            trigger_text = f"Unknown ({log.trigger})"
            if log.trigger == AlertTrigger.PULSE_THRESHOLD.value: trigger_text = "Pulse Threshold"
            if log.trigger == AlertTrigger.SUDDEN_MOVEMENT.value: trigger_text = "Sudden Movement"
            alert_logs.append({
                "log": log,
                "trigger_text": trigger_text
            })

        return render_template('device.html',
                               device=device,
                               status=status,
                               message=message,
                               status_logs=status_logs,
                               alert_logs=alert_logs)
    except Exception:  # Changed from Device.DoesNotExist
        return "Device not found", 404


# --- New Route to Remove a Device ---
@app.route('/device/remove', methods=['POST'])
def remove_device():
    try:
        mac = int(request.form['mac'])
        device = db.Device.get(db.Device.mac == mac)

        # Manually delete all related data to avoid foreign key errors
        db.Status.delete().where(db.Status.device == device).execute()
        db.Location.delete().where(db.Location.device == device).execute()
        db.Alert.delete().where(db.Alert.device == device).execute()

        # Finally, delete the device itself
        device.delete_instance()

        flash(f"Device {device.name} (MAC: {mac}) has been removed.", "success")
    except ValueError:
        flash("Error: Invalid MAC.", "error")
    except db.Device.DoesNotExist:
        flash("Error: Device not found.", "error")
    except Exception as e:
        flash(f"An error occurred: {e}", "error")

    return redirect(url_for('dashboard'))


@app.route('/register_device', methods=['POST'])
def register_device():
    try:
        mac = int(request.form['mac'])
        name = request.form['name']

        # --- Reverted: Device is created by the radio thread on response ---
        # db.Device.create(mac=mac, name=name, version="Pending")
        # --- End of Revert ---

        # Get the queue from the app's config and send command
        radio_queue = app.config.get('RADIO_QUEUE')
        if radio_queue:
            radio_queue.put(('register', (name, mac)))
            flash(f'Successfully registered {name} (MAC: {mac}). Registration signal sent.')
        else:
            flash(f'Successfully registered {name} (MAC: {mac}). (Radio offline - signal NOT sent).')

    # except IntegrityError: # This check is no longer needed here
    #      flash(f'Error: Device MAC {mac} is already registered.')
    except ValueError:
        flash('Error: Device MAC must be an integer.')
    except Exception as e:
        flash(f'An error occurred: {e}')

    return redirect(url_for('dashboard'))


# --- API Endpoints (Updated) ---

@app.route('/api/device/<int:device_mac>/locations')
def api_device_locations(device_mac):
    try:
        device = db.Device.get(db.Device.mac == device_mac)  # <-- Use db. prefix
        query = (db.Location.select()  # <-- Use db. prefix
                 .where(db.Location.device == device)
                 .order_by(db.Location.timestamp))

        locations = []
        for loc in query:
            locations.append({
                "lat": convert_gps_to_signed(loc.latitude, loc.latitude_direction),
                "lon": convert_gps_to_signed(loc.longitude, loc.longitude_direction),
                "ts": loc.timestamp.isoformat()
            })
        return jsonify(locations)

    except Exception:  # Changed from Device.DoesNotExist
        return jsonify({"error": "device not found"}), 404


@app.route('/api/device/<int:device_mac>/latest_pulse')
def api_latest_pulse(device_mac):
    try:
        device = db.Device.get(db.Device.mac == device_mac)  # <-- Use db. prefix
        latest_status = (db.Status.select(db.Status.bpm, db.Status.avg_bpm, db.Status.timestamp)  # <-- Use db. prefix
                         .where(db.Status.device == device)
                         .order_by(db.Status.timestamp.desc())
                         .get())

        return jsonify({
            "bpm": latest_status.bpm,
            "avg_bpm": latest_status.avg_bpm,
            "timestamp": latest_status.timestamp.isoformat()
        })

    except Exception:  # Changed from (Device.DoesNotExist, Status.DoesNotExist)
        return jsonify({"error": "no data"}), 404


@app.route('/debug/add_user', methods=['POST'])
def debug_add_user():
    try:
        username = request.form['username']
        password = request.form['password']
        hashed_password = generate_password_hash(password)
        db.User.create(username=username, password_hash=hashed_password)  # <-- Use db. prefix
        flash(f"Debug: Created user '{username}'.")
    except Exception as e:
        flash(f"Debug Error: {e}")
    return redirect(request.referrer or url_for('dashboard'))


@app.route('/debug/add_status', methods=['POST'])
def debug_add_status():
    try:
        device_mac = int(request.form['device_mac'])
        device = db.Device.get(db.Device.mac == device_mac)  # <-- Use db. prefix
        db.Status.create(  # <-- Use db. prefix
            device=device,
            bpm=int(request.form['bpm']),
            avg_bpm=int(request.form['avg_bpm']),
            flags=int(request.form['flags'])
        )
        flash(f"Debug: Added Status for MAC {device_mac}.")
    except Exception as e:
        flash(f"Debug Error: {e}")
    return redirect(request.referrer or url_for('dashboard'))


@app.route('/debug/add_location', methods=['POST'])
def debug_add_location():
    try:
        device_mac = int(request.form['device_mac'])
        device = db.Device.get(db.Device.mac == device_mac)  # <-- Use db. prefix
        db.Location.create(  # <-- Use db. prefix
            device=device,
            latitude=float(request.form['latitude']),
            latitude_direction=request.form['latitude_direction'],
            longitude=float(request.form['longitude']),
            longitude_direction=request.form['longitude_direction']
        )
        flash(f"Debug: Added Location for MAC {device_mac}.")
    except Exception as e:
        flash(f"Debug Error: {e}")
    return redirect(request.referrer or url_for('dashboard'))


@app.route('/debug/add_alert', methods=['POST'])
def debug_add_alert():
    try:
        device_mac = int(request.form['device_mac'])
        device = db.Device.get(db.Device.mac == device_mac)  # <-- Use db. prefix
        db.Alert.create(  # <-- Use db. prefix
            device=device,
            trigger=int(request.form['trigger'])
        )
        flash(f"Debug: Added Alert for MAC {device_mac}.")
    except Exception as e:
        flash(f"Debug Error: {e}")
    return redirect(request.referrer or url_for('dashboard'))


# --- Database Initialization (Run once) ---

def initialize_database():
    db.db.connect()  # <-- Use db.db
    # Use safe=True to avoid crashing if tables already exist
    db.db.create_tables([db.User, db.Device, db.Status, db.Location, db.Alert], safe=True)  # <-- Use db. prefix

    # Create 'admin' user
    if not db.User.select().where(db.User.username == 'admin').exists():  # <-- Use db. prefix
        print("Creating default admin user...")
        hashed_password = generate_password_hash('password')  # CHANGE THIS
        db.User.create(username='admin', password_hash=hashed_password)  # <-- Use db. prefix
        print("User 'admin' created with password 'password'")

    # Create dummy device
    if not db.Device.select().where(db.Device.mac == 12345678).exists():  # <-- Use db. prefix
        print("Creating dummy test device...")
        dev = db.Device.create(mac=12345678, name="Test Soldier Alpha", version="v1.2")  # <-- Use db. prefix

        # Add dummy data
        db.Status.create(device=dev, bpm=75, avg_bpm=72, flags=0)  # <-- Use db. prefix
        db.Location.create(device=dev, latitude=49.8397, latitude_direction='N', longitude=24.0297,
                           longitude_direction='E')  # <-- Use db. prefix
        print("Dummy device and data created.")

    db.db.close()  # <-- Use db.db
