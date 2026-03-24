#include <WiFi.h>
#include <EEPROM.h>

#include "Config.hh"
#include "Esp32UDPReader.hh"
#include "Esp32UDPWriter.hh"

#include "../msgs/MessageSerializer.hh"
#include "../msgs/Telemetry.hh"
#include "../msgs/TimeSync.hh"

Config config;
unsigned int cycle_count = 0;
double uptime_sec = 0.0;
double offset_sec = 0.0;
double syncd_time_sec = 0.0;

// TCP Server
const int TCP_PORT = 7600;
WiFiServer server(TCP_PORT);

// Serializers - Writer
Telemetry telemetry;
MessageSerializer<Telemetry> telemetry_serializer(&telemetry, MTYPE_TELEMETRY);
// Serializers - Reader
TimeSync timesync;
MessageSerializer<TimeSync> timesync_serializer(MTYPE_TIME_SYNC);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Load persistent config from device EEPROM
  loadConfigFromEEPROM();
  delay(1000);
  
  Serial.println("\n\nESP32 Startup");
  Serial.println("==========================");
  
  // Connect to WiFi
  connectToWiFi();
}

void loop() {
  cycle_count++;
  uptime_sec = millis() / 1000.0;
  syncd_time_sec = uptime_sec + offset_sec;
  printf("\nCycle #: %u  Uptime S: %.2f  Synced Time S: %.2f\n", cycle_count, uptime_sec, syncd_time_sec);

  // Verify the WiFI connection is still alive, if not attempt to reconnect
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi connection lost. Attempting to reconnect...");
    connectToWiFi();
    sleep(5);
  }

  readAndPopulateNetworkMessages();

  populateAndWriteTelemetry();

  delay(50);
}


// Read network input msgs
void readAndPopulateNetworkMessages()
{
  String remote_ip;
  int remote_port;
  int bytes = 0;

  // Keep reading until buffer is empty, always keeping the latest
  unsigned char buffer[MAX_MSG_SIZE];
  while ((bytes = readPacketUDP(7600, buffer, MAX_MSG_SIZE, remote_ip, remote_port)) > 0)
  {
      // 1) Timesync - Check to see if the server has sent a request to sync clocks
      if (timesync_serializer.Read(buffer, bytes, &timesync) == 0)
      {
        setTimeOffset(timesync);
      }
  }
}


// Use timesync to calculate offset between robot clock and server clock
void setTimeOffset(TimeSync &_timesync)
{
  offset_sec = _timesync.mono_seconds_ - uptime_sec;
}


// Populate Telemetry data
void populateAndWriteTelemetry()
{
  telemetry.robot_id_ = config.robot_number;
  telemetry.uptime_sec_ = 0.1f * cycle_count;
  telemetry.pos_.x_ = 1.0f * cycle_count;
  telemetry.pos_.y_ = 2.0f * cycle_count;
  telemetry.vel_.x_ = 0.5f * cycle_count;
  telemetry.vel_.y_ = 0.5f * cycle_count;
  telemetry.acc_.x_ = 0.1f * cycle_count;
  telemetry.acc_.y_ = 0.1f * cycle_count;
  telemetry.heading_ = 45.0f * cycle_count;

  // Serialize telemetry message
  unsigned char buffer[MAX_MSG_SIZE];
  int msg_size = telemetry_serializer.GetMessageSize();
  telemetry_serializer.Write(buffer, msg_size);

  // Send it!
  writePacketUDP("192.168.1.191", config.port, buffer, msg_size);
}


// Load config from EEPROM address 0
void loadConfigFromEEPROM()
{
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.readBytes(0, (uint8_t *) &config, sizeof(config));

  Serial.println("=== CONFIG POPULATED ===");
  Serial.print("SSID: ");
  Serial.println(config.ssid);
  Serial.print("PWD: ");
  Serial.println(config.password);
  Serial.print("Port: ");
  Serial.println(config.port);
  Serial.print("Robot#: ");
  Serial.println(config.robot_number);
  Serial.println("");
  
  // Validate data (optional - check if EEPROM was ever written)
  if (config.port == 0 || config.port > 65535) {
    Serial.println("WARNING: Invalid config in EEPROM. Using defaults.");
    config.port = 22;
    strcpy(config.ssid, "DefaultSSID");
    strcpy(config.password, "DefaultPassword");
    config.robot_number = 0;
  }
}

void connectToWiFi()
{
  Serial.print("Connecting to WiFi: ");
  Serial.println(config.ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid, config.password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}
