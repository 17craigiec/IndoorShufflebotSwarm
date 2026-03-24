#include "ShuffleServer.hh"

#include <iostream>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>


ShuffleServer::ShuffleServer(const std::string &_interface_name, int _server_port, int _verbose)
{
    server_ip_ = GetInterfaceIp(_interface_name);
    server_port_ = _server_port;
    socket_fd_ = -1;
    running_ = false;

    verbose_ = _verbose;
}

ShuffleServer::~ShuffleServer()
{
}


std::string ShuffleServer::GetInterfaceIp(const std::string &_interface_name)
{
    struct ifaddrs *ifaddr, *ifa;
    char ip_address[INET_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1)
    {
        std::cerr << "[Error] Failed to get interface addresses" << std::endl;
        PrintAvailableInterfaces();
        return "";
    }

    std::string result = "";

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
            continue;

        if ((std::strcmp(ifa->ifa_name, _interface_name.c_str()) == 0) &&
            (ifa->ifa_addr->sa_family == AF_INET))
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, ip_address, INET_ADDRSTRLEN);
            result = ip_address;
            break;
        }
    }

    freeifaddrs(ifaddr);
    std::cout << "Selected interface: " << _interface_name << ", IP: " << result << std::endl;
    return result;
}


int ShuffleServer::Start()
{
    if (running_)
    {
        std::cerr << "[Warning] ShuffleServer: Already running" << std::endl;
        return -1;
    }

    // Create UDP socket
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0)
    {
        std::cerr << "[Error] ShuffleServer: Failed to create socket" << std::endl;
        return -1;
    }

    // Set socket to reusable
    int reuse = 1;
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        std::cerr << "[Error] ShuffleServer: Failed to set socket option" << std::endl;
        close(socket_fd_);
        return -1;
    }

    // Bind to port
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Listen on all interfaces
    addr.sin_port = htons(server_port_);

    if (bind(socket_fd_, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "[Error] ShuffleServer: Failed to bind to port " << server_port_ << std::endl;
        close(socket_fd_);
        return -1;
    }

    running_ = true;
    listen_thread_ = std::thread(&ShuffleServer::ListenThread, this);

    std::cout << "[Info] ShuffleServer: Listening on port " << server_port_ << std::endl;
    return 0;
}


void ShuffleServer::Stop()
{
    if (!running_)
        return;

    running_ = false;

    // Force the socket to close (wakes up recvfrom)
    shutdown(socket_fd_, SHUT_RDWR);
    close(socket_fd_);

    if (socket_fd_ >= 0)
    {
        close(socket_fd_);
        socket_fd_ = -1;
    }

    if (listen_thread_.joinable())
    {
        listen_thread_.join();
    }

    std::cout << "[Info] ShuffleServer: Stopped listening" << std::endl;
}


void ShuffleServer::ListenThread()
{
    const int BUFFER_SIZE = 4096;
    unsigned char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int packet_count = 0;

    double t_last_recv = GetMonotonicSec();

    while (running_)
    {
        // Reset fd_set EVERY iteration
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(socket_fd_, &readfds);

        // Set timeout
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;  // 100ms timeout

        bool valid_packet_read = false;
        int select_result = select(socket_fd_ + 1, &readfds, nullptr, nullptr, &tv);

        ssize_t recv_len = 0;
        if (select_result > 0)
        {
            // Receive UDP packet
            recv_len = recvfrom(socket_fd_, buffer, BUFFER_SIZE, MSG_DONTWAIT,
                                (struct sockaddr *)&client_addr, &addr_len);

            if (recv_len < 0)
            {
                std::cerr << "[Error] ShuffleServer: recvfrom failed" << std::endl;
            }

            // Validation: If select returned > 0 but recv_len is 0, this likely means the socket was shutdown while waiting in recvfrom.
            if (recv_len == 0)
            {
                // This can happen if the socket is shutdown while waiting in recvfrom
                if (verbose_ >= 3)
                {
                    std::cout << "[Info] ShuffleServer: recvfrom returned 0 bytes, likely due to shutdown" << std::endl;
                }
            }

            if (recv_len > 0)
            {
                valid_packet_read = true;
            }
        }

        // If you made it here, we have a new valid packet! Try to parse it!
        char client_ip[INET_ADDRSTRLEN];
        client_ip[0] = '\0';
        int client_port = -1;
        if (valid_packet_read)
        {
            packet_count++;

            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            client_port = ntohs(client_addr.sin_port);

            if (verbose_ >= 3)
            {
                // Print packet info
                std::cout << "\n========================================" << std::endl;
                std::cout << "Packet #" << packet_count << std::endl;
                std::cout << "From: " << client_ip << ":" << client_port << std::endl;
                std::cout << "Length: " << recv_len << " bytes" << std::endl;
                std::cout << "----------------------------------------" << std::endl;

                // Print data as hex and ASCII
                std::cout << "Hex Dump:" << std::endl;
                for (int i = 0; i < recv_len; i++)
                {
                    if (i % 16 == 0)
                    {
                        std::cout << std::setw(4) << std::setfill('0') << i << ": ";
                    }

                    std::cout << std::hex << std::setw(2) << std::setfill('0') 
                            << (int)buffer[i] << " ";

                    if ((i + 1) % 16 == 0)
                    {
                        std::cout << std::endl;
                    }
                }
                std::cout << std::dec << std::endl;

                // Print as ASCII (printable chars only)
                std::cout << "ASCII:" << std::endl;
                for (int i = 0; i < recv_len; i++)
                {
                    if (i % 64 == 0)
                        std::cout << std::setw(4) << std::setfill('0') << i << ": ";

                    if (buffer[i] >= 32 && buffer[i] <= 126)
                        std::cout << (char)buffer[i];
                    else
                        std::cout << ".";

                    if ((i + 1) % 64 == 0)
                        std::cout << std::endl;
                }
                std::cout << std::endl;
                std::cout << "========================================" << std::endl << std::endl;
            }

            PopulateInputs(std::string(client_ip), buffer, recv_len);
        }

        if (verbose_ >= 1)
        {
            std::cout << "Cycle took " << std::fixed << std::setprecision(4) << (GetMonotonicSec() - t_last_recv) << "s  ";
            if (client_port != -1)
            {
                std::cout << "Msg From: " << client_ip << ":" << client_port << "  Bytes: " << recv_len;
            }
            std::cout << std::endl;
        }

        t_last_recv = GetMonotonicSec();
    } // End while(running_)

    std::cout << "[Info] ShuffleServer: Listen thread exiting" << std::endl;
}

bool ShuffleServer::IsRunning()
{
    return running_;
}

int ShuffleServer::SendTimesyncToClients()
{
    // Loop through the mapping of clients
    for (const auto &[client_ip, connection] : robot_connections_)
    {
        // Set the timesync message with current server time
        timesync_.mono_seconds_ = GetMonotonicSec();

        // Serialize timesync message
        unsigned char buffer[MAX_MSG_SIZE];
        timesync_serializer_.Write(buffer, MAX_MSG_SIZE);

        // Send it!
        struct sockaddr_in client_addr;           // Socket address structure
        std::memset(&client_addr, 0, sizeof(client_addr)); // Clear it (set all to 0)

        client_addr.sin_family = AF_INET;         // IPv4
        inet_pton(AF_INET, 
                  client_ip.c_str(),              // Convert "192.168.1.101"
                  &client_addr.sin_addr);         // Into binary IP address

        client_addr.sin_port = htons(connection.port_); // Set port (7600)
                                                        // htons = host to network byte order

        int msg_size = timesync_serializer_.GetMessageSize();
        sendto(socket_fd_, buffer, msg_size, 0,
                (struct sockaddr *)&client_addr, sizeof(client_addr));
    }
    return 0;
}

int ShuffleServer::PopulateInputs(std::string _client_ip, const unsigned char *_data, unsigned int _len)
{
    // Lock the mutex to protect shared data structures
    // It is safe to update the robot_connections_ map here.
    std::lock_guard<std::mutex> lock(mutex_);

    // Check if the client ip exists in the robot connections map, if not add it
    if (robot_connections_.find(_client_ip) == robot_connections_.end())
    {
        RobotConnection new_connection;
        new_connection.ip_address_ = _client_ip;
        new_connection.port_ = server_port_;
        // Populate this new entry
        robot_connections_[_client_ip] = new_connection;
    }
    else
    {
        // If the client already exists, we can update the last contact time
        robot_connections_[_client_ip].last_contact_mono_sec_ = 0; // TODO: Set to current time
    }

    // Check all inputs to see if this packet matches any expected message types
    Telemetry tmp_telemetry;
    int ret_val = telemetry_serializer_.Read(_data, _len, &tmp_telemetry);
    if (ret_val == 0)
    {
        if (verbose_ >= 2)
        {
            std::cout << "[Info] Parsed Telemetry message successfully!" << std::endl;
            printTelemetry(tmp_telemetry);
        }

        // Update the robot connection info with the new telemetry
        robot_connections_[_client_ip].telemetry_ = tmp_telemetry;
    }
    else
    {
        std::cerr << "[Warning] Failed to parse message from client IP: " << _client_ip 
                  << ". Data may not match expected message formats." << std::endl;
        // TODO Get rid of this eventually, exit early for testing.
        return -1;
    }

    return 0;
}


// Helper functions
void PrintAvailableInterfaces(void)
{
    struct ifaddrs *ifaddr, *ifa;
    char ip_address[INET_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1)
    {
        std::cerr << "[Error] Failed to get interface addresses" << std::endl;
        return;
    }

    std::cout << "Available network interfaces:" << std::endl;

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, ip_address, INET_ADDRSTRLEN);
            std::cout << "  " << ifa->ifa_name << ": " << ip_address << std::endl;
        }
    }

    freeifaddrs(ifaddr);
}

double GetMonotonicSec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}
