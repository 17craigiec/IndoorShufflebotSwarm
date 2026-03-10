#!/bin/bash

BAUD=115200
USBPORT=/dev/ttyUSB0

# Get script directory (not current working directory)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Go into the client dir
cd $SCRIPT_DIR
# Exit on any error
set -e

echo "=== Cleaning old builds ==="
rm -rf build/

# Compile
echo "=== Compiling ==="
arduino-cli compile --fqbn esp32:esp32:esp32 --build-path ./build .
# Upload
echo "=== Uploading ==="
arduino-cli upload -p $USBPORT --fqbn esp32:esp32:esp32 --build-path ./build .
# Monitor Serial
echo "=== Opening Serial Monitor (Ctrl+C to exit) ==="
arduino-cli monitor -p $USBPORT --config baudrate=$BAUD
