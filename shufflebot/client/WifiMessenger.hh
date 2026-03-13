#include <EEPROM.h>
#include <WiFi.h>


#include "Config.hh"

class WifiMessenger
{
private:
    /* data */
public:
    WifiMessenger(Config &_config);
    ~WifiMessenger();
};

WifiMessenger::WifiMessenger(Config &_config)
{
}

WifiMessenger::~WifiMessenger()
{
}
