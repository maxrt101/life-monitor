from station import config, server, radio, db, tests
from station.utils import logger
import threading
import argparse
import queue


# Project version
__VERSION__ = '0.1'

# Use an Event to signal threads to shut down
shutdown_event = threading.Event()

# Create a thread-safe queue for Web -> Radio commands
radio_command_queue = queue.Queue()


def server_thread(port: int):
    logger.info("Starting Flask server thread...")
    try:
        # 'Inject' the command queue into the Flask app config
        server.app.config['RADIO_QUEUE'] = radio_command_queue

        # Must use use_reloader=False when in a thread
        server.app.run(debug=True, use_reloader=False, host='0.0.0.0', port=port)
    except Exception as e:
        logger.error(f"Flask server crashed: {e}")
    finally:
        logger.info("Flask server shutting down")


def radio_thread(spidev: str):
    logger.info("Starting Radio network thread...")

    try:
        # Open a new DB connection for this thread
        db.conn.connect(reuse_if_open=True)

        net = radio.Network(radio.create_driver(spidev), config.CONFIG_RADIO_KEY, config.CONFIG_RADIO_DEFAULT_KEY)
        logger.info("Radio network initialized")

        # Loop until the main thread signals shutdown
        while not shutdown_event.is_set():
            # Check for commands from the web server
            try:
                command, args = radio_command_queue.get_nowait()
                if command == 'register':
                    name, mac = args
                    logger.info(f"Radio Thread: Received registration command for {name} ({mac})")
                    net.start_registration(name, mac)
            except queue.Empty:
                # No command, continue
                pass
            except Exception as e:
                logger.error(f"Radio command processing error: {e}")

            net.cycle()

            # Use the event's wait() method instead of sleep()
            # This waits for timeout OR until the event is set
            # so shutdown is almost instantaneous
            shutdown_event.wait(timeout=config.CONFIG_RADIO_THREAD_CYCLE_PERIOD)

    except Exception as e:
        logger.error(f"Radio thread crashed: {e}")
    finally:
        # Allways close the thread-local DB connection
        db.deinit()
        logger.info("Radio thread shut down")


def main():
    logger.info(f'LifeMonitorStation v{__VERSION__}')

    parser = argparse.ArgumentParser(prog='LifeMonitorStation')

    parser.add_argument('-p', '--port', type=int, help='Server port', dest='port', default=8086)
    parser.add_argument('-s', '--spidev', type=str, help='SPI Dev path', dest='spidev', default=config.CONFIG_RADIO_SX1278_SPIDEV)
    parser.add_argument('-t', '--tests', action='store_true', help='Run tests', dest='tests', default=False)

    args = parser.parse_args()

    if args.tests:
        tests.run()
        # Exit after tests
        return

    db.init()

    # Make the server a 'daemon' thread
    # This means it will be killed automatically when the main thread exits
    # This is fine for the web server
    srv_thread = threading.Thread(target=server_thread, args=(args.port, ), daemon=True)

    # The radio thread is NOT a daemon, because it should
    # shut down gracefully and close its database connection
    rf_thread = threading.Thread(target=radio_thread, args=(args.spidev, ))

    logger.info("Starting services...")

    srv_thread.start()
    rf_thread.start()

    logger.info("Services running. Press Ctrl+C to stop")

    try:
        # The main thread will wait here until the radio thread exits.
        # It will only exit when the shutdown_event is set
        rf_thread.join()
    except KeyboardInterrupt:
        # Print newline after '^C'
        print()
        logger.info("Caught Ctrl+C. Shutting down...")

        # Signal threads to stop
        shutdown_event.set()

        # Wait for the radio thread to finish its loop and close
        rf_thread.join()

    logger.info("Shutdown complete.")


if __name__ == '__main__':
    main()