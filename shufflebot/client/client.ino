#include <WiFi.h>
#include <EEPROM.h>

#include "config.hh"

Config config;

// TCP Server
const int TCP_PORT = 22;
WiFiServer server(TCP_PORT);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Load persistent config from device EEPROM
  loadConfigFromEEPROM();
  delay(1000);
  
  Serial.println("\n\nESP32 TCP Ping Responder");
  Serial.println("==========================");
  
  // Connect to WiFi
  connectToWiFi();
  
  // Start TCP server
  startTCPServer();
}

void loop() {
  // Check for incoming TCP connections
  handleTCPConnections();
  
  delay(100);
}

// Load config from EEPROM address 0
void loadConfigFromEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.readBytes(0, (uint8_t *) &config, sizeof(config));

  Serial.println("=== CONFIG POPULATED ===");
  Serial.print("SSID: ");
  Serial.println(config.ssid);
  Serial.print("PWD: ");
  Serial.println(config.password);
  Serial.print("Port: ");
  Serial.println(config.tcp_port);
  Serial.print("Robot#: ");
  Serial.println(config.robot_number);
  Serial.println("");
  
  // Validate data (optional - check if EEPROM was ever written)
  if (config.tcp_port == 0 || config.tcp_port > 65535) {
    Serial.println("WARNING: Invalid config in EEPROM. Using defaults.");
    config.tcp_port = 22;
    strcpy(config.ssid, "DefaultSSID");
    strcpy(config.password, "DefaultPassword");
    config.robot_number = 0;
  }
}

void connectToWiFi() {
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

void startTCPServer() {
  server.begin(TCP_PORT);
  Serial.print("TCP Server started on port ");
  Serial.println(TCP_PORT);
  Serial.println("Waiting for ping requests...");
}

void handleTCPConnections() {
  // Check for new client connections
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client connected!");
    
    // Read incoming data
    String request = "";
    while (client.connected() && client.available()) {
      char c = client.read();
      request += c;
    }
    
    // Print message
    Serial.println("hello ping");
    Serial.print("  Received: ");
    Serial.println(request);
    
    // Send response
    client.print("hello ping\n");
    
    Serial.println("  Response sent!");
    
    // Close the connection
    client.stop();
    Serial.println("Client disconnected\n");
  }
}
