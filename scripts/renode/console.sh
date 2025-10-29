#!/usr/bin/env bash

# Delay between checks of UART console file presence
DELAY="0.1"

# UART console file path
FILE="/tmp/renode-life-monitor-console"

# Starts minicom and returns exit code when done
start() {
  minicom -c on -D $FILE
  return $?
}

# Stops minicom, if successful (suitable minicom process was found and killed)
# will reset TTY and print reconnection message
stop() {
  pkill -9 -P $$ "minicom"

  if [ $? == 0 ]; then
    reset
    echo "Connection lost, will repeat every ${DELAY}s"
  fi
}

# Watch for UART file presence, if not present, will stop minicom
watch() {
  while true; do
    if [ ! -L "$FILE" ]; then
      stop
    fi

    sleep $DELAY
  done
}

# Called on SIGINT (^C) to kill all child processed
cleanup() {
  kill -- -$$
  exit
}

# Main function
main() {
  echo "Will try to connect every ${DELAY}ms"

  # Set handler for SIGINT (^C)
  trap cleanup SIGINT

  # Start watch "thread" in background
  watch &

  while true; do
    # Wait for UART console file to become ready
    while [ ! -L "$FILE" ]; do
      sleep 0.05
    done

    # Start minicom
    start

    # Minicom returns 0 if user gracefully exited it
    if [ $? == 0 ]; then
      echo "Done"
      exit
    fi

    sleep $DELAY
  done
}

main
