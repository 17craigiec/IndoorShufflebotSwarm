#ifndef ESP32_UDP_READER_HH
#define ESP32_UDP_READER_HH

#include <WiFi.h>
#include <WiFiUdp.h>

// Listen for UDP packet on a specific port
// Returns number of bytes received, 0 if no packet, -1 on error
int readPacketUDP(int _port, unsigned char *_buffer, unsigned int _max_len, 
                  String &_remote_ip, int &_remote_port)
{
    static WiFiUDP udp;
    static bool initialized = false;
    static int current_port = -1;

    // Initialize UDP listener on first call
    if (!initialized || current_port != _port)
    {
        udp.stop();
        if (!udp.begin(_port))
        {
            Serial.println("[Error] readPacketUDP: Failed to begin UDP on port " + String(_port));
            return -1;
        }
        initialized = true;
        current_port = _port;
        Serial.println("[Info] readPacketUDP: Listening on port " + String(_port));
    }

    // Check if packet is available
    int packet_size = udp.parsePacket();
    
    if (packet_size == 0)
    {
        return 0;  // No packet available
    }

    if (packet_size < 0)
    {
        Serial.println("[Error] readPacketUDP: Error parsing packet");
        return -1;
    }

    // Packet too large for buffer
    if (packet_size > (int)_max_len)
    {
        Serial.print("[Warning] readPacketUDP: Packet too large (");
        Serial.print(packet_size);
        Serial.print(" > ");
        Serial.print(_max_len);
        Serial.println(")");
        return -1;
    }

    // Get remote IP and port
    _remote_ip = udp.remoteIP().toString();
    _remote_port = udp.remotePort();

    // Read the packet
    int bytes_read = udp.read(_buffer, _max_len);

    Serial.print("[Info] readPacketUDP: Received ");
    Serial.print(bytes_read);
    Serial.print(" bytes from ");
    Serial.print(_remote_ip);
    Serial.print(":");
    Serial.println(_remote_port);

    return bytes_read;
}

#endif // ESP32_UDP_READER_HH