#include <iostream>
#include <iomanip>
#include <cstring>
#include <sys/socket.h>
#include <mutex>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include <vector>
#include <map>

#include "MessageSerializer.hh"
#include "Telemetry.hh"
#include "TimeSync.hh"


struct RobotConnection
{
    std::string ip_address_;
    int port_;

    Telemetry telemetry_;
    double last_contact_mono_sec_;
};


class ShuffleServer
{
private:
    std::string server_ip_;
    int server_port_;
    int socket_fd_;
    bool running_;

    int verbose_ = 0;

    std::thread listen_thread_;
    mutable std::mutex mutex_;
    std::map<std::string, RobotConnection> robot_connections_;

    // Input Msgs
    MessageSerializer<Telemetry> telemetry_serializer_ = MessageSerializer<Telemetry>(MTYPE_TELEMETRY);

    // Output Msgs
    TimeSync timesync_;
    MessageSerializer<TimeSync> timesync_serializer_ = MessageSerializer<TimeSync>(&timesync_, MTYPE_TIME_SYNC);
    
public:
    ShuffleServer(const std::string &interface_name, int _server_port, int _verbose=0);
    ~ShuffleServer();

    std::string GetInterfaceIp(const std::string &interface_name);

    // Server Network Functions
    int Start();
    void Stop();
    void ListenThread();
    bool IsRunning();

    // Message handling functions
    int PopulateInputs(std::string _client_ip, const unsigned char *_data, unsigned int _len);
    // Send timesync to clients
    int SendTimesyncToClients();
};


// Helpers
void PrintAvailableInterfaces(void);
double GetMonotonicSec(void);