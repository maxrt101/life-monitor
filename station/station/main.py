from station import config, server, radio, db, tests
from station.utils import logger
import threading
import argparse
import queue

__VERSION__ = '0.1'

# Use an Event to signal threads to shut down
shutdown_event = threading.Event()
# Create a thread-safe queue for Web -> Radio commands
radio_command_queue = queue.Queue()


def server_thread():
    """Runs the Flask server."""
    logger.info("Starting Flask server thread...")
    try:
        # "Inject" the command queue into the Flask app config
        server.app.config['RADIO_QUEUE'] = radio_command_queue

        # Must use use_reloader=False when in a thread
        server.app.run(debug=True, use_reloader=False, host='0.0.0.0', port=8086)
    except Exception as e:
        logger.error(f"Flask server crashed: {e}")
    finally:
        logger.info("Flask server shutting down.")


def radio_thread():
    """Runs the LoRa network cycle."""
    logger.info("Starting Radio network thread...")

    try:
        # Open a new DB connection for this thread
        db.db.connect(reuse_if_open=True)

        # Initialize your network driver
        net = radio.Network(radio.create_driver(), config.CONFIG_RADIO_KEY, config.CONFIG_RADIO_DEFAULT_KEY)
        logger.info("Radio network initialized.")

        # Loop until the main thread signals shutdown
        while not shutdown_event.is_set():

            # --- Check for commands from the web server ---
            try:
                command, args = radio_command_queue.get_nowait()
                if command == 'register':
                    name, mac = args
                    logger.info(f"Radio Thread: Received registration command for {name} ({mac})")
                    logger.info(f"Radio Thread: Registration for {mac} initiated.")
                    net.start_registration(name, mac)
            except queue.Empty:
                pass  # No command, continue
            except Exception as e:
                logger.error(f"Radio command processing error: {e}")

            net.cycle()

            # Use the event's wait() method instead of sleep().
            # This waits for 0.5s OR until the event is set,
            # so shutdown is almost instantaneous.
            shutdown_event.wait(timeout=0.5)

    except Exception as e:
        logger.error(f"Radio thread crashed: {e}")
    finally:
        # CRITICAL: Always close the thread-local DB connection
        if not db.db.is_closed():
            db.db.close()
        logger.info("Radio thread shut down")


def main():
    logger.info(f'LifeMonitorStation v{__VERSION__}')

    parser = argparse.ArgumentParser(prog='LifeMonitorStation')
    parser.add_argument('-t', '--tests', action='store_true', help='Run tests', dest='tests', default=False)
    args = parser.parse_args()

    if args.tests:
        tests.run()
        return  # Exit after tests

    db.init()

    # We make the server a 'daemon' thread.
    # This means it will be killed automatically when the main thread exits.
    # This is fine for the web server.
    srv_thread = threading.Thread(target=server_thread, daemon=True)

    # The radio thread is NOT a daemon, because we want it to
    # shut down gracefully and close its database connection.
    rf_thread = threading.Thread(target=radio_thread)

    logger.info("Starting services...")

    srv_thread.start()
    rf_thread.start()

    logger.info("Services running. Press Ctrl+C to stop.")

    try:
        # The main thread will wait here until the radio thread exits.
        # It will only exit when we set the shutdown_event.
        rf_thread.join()
    except KeyboardInterrupt:
        logger.info("\nCaught Ctrl+C. Shutting down...")

        # --- Signal threads to stop ---
        shutdown_event.set()

        # Now we wait for the radio thread to finish its loop and close
        rf_thread.join()

    logger.info("Shutdown complete.")


if __name__ == '__main__':
    main()