#!/bin/bash

# save_config.sh - Configure and upload ESP32

if [ $# -lt 4 ]; then
  echo "Usage: $0 <SSID> <PASSWORD> <PORT> <ROBOT_NUMBER>"
  echo "Example: $0 RockLobster mypass123 22 1"
  exit 1
fi

SSID=$1
PASSWORD=$2
PORT=$3
ROBOT_NUM=$4

# Get script directory (not current working directory)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

SKETCH_DIR=$SCRIPT_DIR/config_upload/
USBPORT=/dev/ttyUSB0
FQBN=esp32:esp32:esp32
BAUD=115200

echo "=== Configuring ESP32 ==="
echo "SSID: $SSID"
echo "Password: $PASSWORD"
echo "Port: $PORT"
echo "Robot#: $ROBOT_NUM"

# Prompt to proceed
read -p "Proceed with upload? [Y/n] " -n 1 -r
echo  # New line after response
if [[ $REPLY =~ ^[Nn]$ ]]; then
  echo "Cancelled."
  exit 0
fi

# Create a temporary config sketch
mkdir -p $SKETCH_DIR
cat > $SKETCH_DIR/config_upload.ino << EOF
#include <EEPROM.h>
#include "../config.hh"

void setup() {
  Serial.begin($BAUD);
  delay(1000);
  
  EEPROM.begin(EEPROM_SIZE);
  
  // Create config
  Config config;
  config.tcp_port = $PORT;
  config.robot_number = $ROBOT_NUM;
  
  // Copy strings (safely)
  strncpy(config.ssid, "$SSID", 31);
  config.ssid[31] = '\0';
  
  strncpy(config.password, "$PASSWORD", 31);
  config.password[31] = '\0';
  
  // Write to EEPROM
  EEPROM.writeBytes(0, (uint8_t *) &config, sizeof(config));
  EEPROM.commit();
  
  for(int i = 0; i < 100; i++){
    delay(1000);
    Serial.println("=== CONFIG SAVED ===");
    Serial.print("SSID: ");
    Serial.println(config.ssid);
    Serial.print("PWD: ");
    Serial.println(config.password);
    Serial.print("Port: ");
    Serial.println(config.tcp_port);
    Serial.print("Robot#: ");
    Serial.println(config.robot_number);
    Serial.println("");
  }
}

void loop() {}
EOF

# Compile
echo ""
echo "=== Compiling ==="
rm -rf $SKETCH_DIR/build/
arduino-cli compile --fqbn $FQBN --build-path $SKETCH_DIR/build $SKETCH_DIR || exit 1

# Upload
echo ""
echo "=== Uploading Config ==="
arduino-cli upload -p $USBPORT --fqbn $FQBN --build-path $SKETCH_DIR/build $SKETCH_DIR || exit 1

# Remove the code - dont need it now
rm -r $SKETCH_DIR

# Monitor to see confirmation
echo ""
echo "=== Monitoring (Ctrl+C to exit) ==="
arduino-cli monitor -p $USBPORT --config baudrate=$BAUD