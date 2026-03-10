#define EEPROM_SIZE 512

struct Config {
  int tcp_port;
  char ssid[32];
  char password[32];
  int robot_number;
};