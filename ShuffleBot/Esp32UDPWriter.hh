#ifndef ESP32_UDP_WRITER_HH
#define ESP32_UDP_WRITER_HH

#include <WiFi.h>
#include <WiFiUdp.h>

// Send UDP packet from ESP32
// Returns 0 on success, -1 on failure
int writePacketUDP(String _server_ip, int _port, unsigned char *_data, unsigned int _length)
{
    WiFiUDP udp;
    
    // Validate inputs
    if (_data == nullptr || _length == 0)
    {
        Serial.println("[Error] writePacketUDP: Invalid data or length");
        return -1;
    }
    
    if (_port <= 0 || _port > 65535)
    {
        Serial.println("[Error] writePacketUDP: Invalid port number");
        return -1;
    }
    
    // Begin UDP
    if (!udp.begin(_port))
    {
        Serial.println("[Error] writePacketUDP: Failed to begin UDP");
        return -1;
    }
    
    // Connect to remote host
    if (!udp.beginPacket(_server_ip.c_str(), _port))
    {
        Serial.println("[Error] writePacketUDP: Failed to begin packet");
        udp.stop();
        return -1;
    }
    
    // Write data
    size_t written = udp.write(_data, _length);
    
    if (written != _length)
    {
        Serial.print("[Warning] writePacketUDP: Wrote ");
        Serial.print(written);
        Serial.print(" bytes, expected ");
        Serial.println(_length);
    }
    
    // Send packet
    if (!udp.endPacket())
    {
        Serial.println("[Error] writePacketUDP: Failed to send packet");
        udp.stop();
        return -1;
    }
    
    udp.stop();
    
    Serial.print("[Info] writePacketUDP: Sent ");
    Serial.print(_length);
    Serial.print(" bytes to ");
    Serial.print(_server_ip);
    Serial.print(":");
    Serial.println(_port);
    
    return 0;
}

#endif // ESP32_UDP_WRITER_HH