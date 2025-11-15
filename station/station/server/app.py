from flask import Flask, render_template, request, redirect, url_for, session, flash, jsonify, g
from werkzeug.security import generate_password_hash, check_password_hash
from station.radio import StatusFlags, AlertTrigger
from station.utils import parse_int
from station import db, config
import datetime


app = Flask('LifeMonitorStation', template_folder='station/server/templates')
app.config['SECRET_KEY'] = config.CONFIG_APP_SESSION_KEY


def get_device_status(device: int) -> (str, str):
    try:
        # 1. Check for recent critical alerts
        recent_alert = (
            db.Alert.select()
             .where(db.Alert.device == device)
             .order_by(db.Alert.timestamp.desc())
             .get_or_none()
        )

        if recent_alert:
            # Check if alert is very recent (e.g., last 5 mins)
            time_diff = datetime.datetime.now() - recent_alert.timestamp
            if time_diff.total_seconds() < 300:
                # Map trigger codes to messages
                trigger_msg = f'TRIGGER={recent_alert.trigger}'
                if recent_alert.trigger == AlertTrigger.PULSE_THRESHOLD.value: trigger_msg = 'Pulse Alert'
                if recent_alert.trigger == AlertTrigger.SUDDEN_MOVEMENT.value: trigger_msg = 'Sudden Movement Detected'
                return 'CRITICAL', f'Alert: {trigger_msg}'

        # 2. Check latest status report
        latest_status = (
            db.Status.select()
             .where(db.Status.device == device)
             .order_by(db.Status.timestamp.desc())
             .get_or_none()
        )

        if not latest_status:
            return 'WARNING', 'No Status Data'

        # 3. Check for offline (no status in 2 minutes)
        time_diff = datetime.datetime.now() - latest_status.timestamp
        if time_diff.total_seconds() > 300:
            return 'CRITICAL', 'Offline (No Signal)'

        # 4. Check status flags
        if latest_status.flags & StatusFlags.PULSE_SENSOR_FAILURE:
            return 'WARNING', f'Pulse Sensor Failure ({latest_status.bpm} BPM)'
        if latest_status.flags & StatusFlags.ACCEL_SENSOR_FAILURE:
            return 'WARNING', f'Accel Sensor Failure ({latest_status.bpm} BPM)'
        if latest_status.flags & StatusFlags.GPS_FAILURE:
            return 'WARNING', f'GPS Failure ({latest_status.bpm} BPM)'

        # 5. Check pulse
        if latest_status.bpm == 0:
            return 'CRITICAL', 'No Pulse Detected'
        if latest_status.bpm < 40 or latest_status.bpm > 180:
            return 'WARNING', f'Abnormal Pulse ({latest_status.bpm} BPM)'

        # If all checks pass:
        return 'OK', f'Nominal ({latest_status.bpm} BPM)'

    except Exception as e:
        return 'WARNING', f'Error ({e})'


def convert_gps_to_signed(value: float, direction: str) -> float:
    # Converts (12.34, 'S') to -12.34
    if direction in ['S', 'W']:
        return -value
    return value


def device_to_dict(device: db.Device, status: str, message: str) -> dict:
    return {
        'mac':     device.mac,
        'mac_hex': f'0x{device.mac:X}',
        'name':    device.name,
        'version': device.version,
        'status':  status,
        'message': message
    }


########## Authentication ##########

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
        except Exception:
            flash('Invalid username or password')
    return render_template('login.html')


@app.route('/logout')
def logout():
    session.pop('user_id', None)
    session.pop('username', None)
    return redirect(url_for('login'))


########## Protected App Routes ##########

@app.before_request
def before_request_handler():
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
    for device in g.all_devices:
        status, message = get_device_status(device)
        devices.append(device_to_dict(device, status, message))
    return render_template('dashboard.html', devices=devices, station_mac=f'0x{config.CONFIG_STATION_MAC:X}')


@app.route('/device/<int:device_mac>')
def device_detail(device_mac):
    try:
        device = db.Device.get(db.Device.mac == device_mac)
        status, message = get_device_status(device)

        # Get recent logs
        status_logs_query = (
            db.Status.select()
             .where(db.Status.device == device)
             .order_by(db.Status.timestamp.desc())
             .limit(50)
        )

        alert_logs_query = (
            db.Alert.select()
             .where(db.Alert.device == device)
             .order_by(db.Alert.timestamp.desc())
             .limit(50)
        )

        # Process logs to include descriptive text
        status_logs = []
        for log in status_logs_query:
            flags_list = []

            if log.flags & StatusFlags.PULSE_SENSOR_FAILURE: flags_list.append('PULSE FAIL')
            if log.flags & StatusFlags.ACCEL_SENSOR_FAILURE: flags_list.append('ACCEL FAIL')
            if log.flags & StatusFlags.GPS_FAILURE:          flags_list.append('GPS FAIL')

            status_logs.append({
                'log': log,
                'flags_text': ', '.join(flags_list) or 'OK'
            })

        alert_logs = []
        for log in alert_logs_query:
            trigger_text = f'Unknown ({log.trigger})'

            if log.trigger == AlertTrigger.PULSE_THRESHOLD.value: trigger_text = 'Pulse Threshold'
            if log.trigger == AlertTrigger.SUDDEN_MOVEMENT.value: trigger_text = 'Sudden Movement'

            alert_logs.append({
                'log': log,
                'trigger_text': trigger_text
            })

        return render_template('device.html',
            device=device_to_dict(device, status, message),
            status_logs=status_logs,
            alert_logs=alert_logs
        )
    except Exception:
        return 'Device not found', 404


@app.route('/device/remove', methods=['POST'])
def remove_device():
    try:
        mac = parse_int(request.form['mac'])
        device = db.Device.get(db.Device.mac == mac)

        # Manually delete all related data to avoid foreign key errors
        db.Status.delete().where(db.Status.device == device).execute()
        db.Location.delete().where(db.Location.device == device).execute()
        db.Alert.delete().where(db.Alert.device == device).execute()

        # Finally, delete the device itself
        device.delete_instance()

        flash(f'Device {device.name} (MAC: {mac}) has been removed', 'success')
    except ValueError:
        flash('Error: Invalid MAC', 'error')
    except Exception as e:
        flash(f'An error occurred: {e}', 'error')

    return redirect(url_for('dashboard'))


@app.route('/register_device', methods=['POST'])
def register_device():
    try:
        mac = int(request.form['mac'])
        name = request.form['name']

        # Get the queue from the app's config and send command
        radio_queue = app.config.get('RADIO_QUEUE')
        if radio_queue:
            radio_queue.put(('register', (name, mac)))
            flash(f'Successfully registered {name} (MAC: {mac}). Registration signal sent')
        else:
            flash(f'Successfully registered {name} (MAC: {mac}). (Radio offline - signal NOT sent)')

    except ValueError:
        flash('Error: Device MAC must be an integer')
    except Exception as e:
        flash(f'An error occurred: {e}')

    return redirect(url_for('dashboard'))


########## API Endpoints ##########

@app.route('/api/device/<int:device_mac>/locations')
def api_device_locations(device_mac):
    try:
        device = db.Device.get(db.Device.mac == device_mac)
        query = (
            db.Location.select()
             .where(db.Location.device == device)
             .order_by(db.Location.timestamp)
        )

        locations = []
        for loc in query:
            locations.append({
                'lat': convert_gps_to_signed(loc.latitude, loc.latitude_direction),
                'lon': convert_gps_to_signed(loc.longitude, loc.longitude_direction),
                'ts':  loc.timestamp.isoformat()
            })
        return jsonify(locations)

    except Exception:
        return jsonify({'error': 'device not found'}), 404


@app.route('/api/device/<int:device_mac>/latest_pulse')
def api_latest_pulse(device_mac):
    try:
        device = db.Device.get(db.Device.mac == device_mac)
        latest_status = (
            db.Status.select(db.Status.bpm, db.Status.avg_bpm, db.Status.timestamp)
             .where(db.Status.device == device)
             .order_by(db.Status.timestamp.desc())
             .get()
        )

        return jsonify({
            'bpm': latest_status.bpm,
            'avg_bpm': latest_status.avg_bpm,
            'timestamp': latest_status.timestamp.isoformat()
        })

    except Exception:
        return jsonify({'error': 'no data'}), 404


@app.route('/debug/add_user', methods=['POST'])
def debug_add_user():
    try:
        username = request.form['username']
        password = request.form['password']
        hashed_password = generate_password_hash(password)
        db.User.create(username=username, password_hash=hashed_password).save()
        flash(f'Debug: Created user \'{username}\'')
    except Exception as e:
        flash(f'Debug Error: {e}')
    return redirect(request.referrer or url_for('dashboard'))


@app.route('/debug/add_device', methods=['POST'])
def debug_add_device():
    try:
        mac = parse_int(request.form['mac'])
        name = request.form['name']
        version = request.form['version']

        db.Device.create(mac=mac, name=name, version=version).save()
        flash(f'Debug: Created device \'{name}\' {hex(mac)}')
    except Exception as e:
        flash(f'Debug Error: {e}')
    return redirect(request.referrer or url_for('dashboard'))


@app.route('/debug/add_status', methods=['POST'])
def debug_add_status():
    try:
        device_mac = int(request.form['device_mac'])
        device = db.Device.get(db.Device.mac == device_mac)
        db.Status.create(
            device=device,
            bpm=int(request.form['bpm']),
            avg_bpm=int(request.form['avg_bpm']),
            flags=int(request.form['flags'])
        ).save()
        flash(f'Debug: Added Status for MAC {device_mac}')
    except Exception as e:
        flash(f'Debug Error: {e}')
    return redirect(request.referrer or url_for('dashboard'))


@app.route('/debug/add_location', methods=['POST'])
def debug_add_location():
    try:
        device_mac = int(request.form['device_mac'])
        device = db.Device.get(db.Device.mac == device_mac)
        db.Location.create(
            device=device,
            latitude=float(request.form['latitude']),
            latitude_direction=request.form['latitude_direction'],
            longitude=float(request.form['longitude']),
            longitude_direction=request.form['longitude_direction']
        ).save()
        flash(f'Debug: Added Location for MAC {device_mac}')
    except Exception as e:
        flash(f'Debug Error: {e}')
    return redirect(request.referrer or url_for('dashboard'))


@app.route('/debug/add_alert', methods=['POST'])
def debug_add_alert():
    try:
        device_mac = int(request.form['device_mac'])
        device = db.Device.get(db.Device.mac == device_mac)
        db.Alert.create(
            device=device,
            trigger=int(request.form['trigger'])
        ).save()
        flash(f'Debug: Added Alert for MAC {device_mac}')
    except Exception as e:
        flash(f'Debug Error: {e}')
    return redirect(request.referrer or url_for('dashboard'))
