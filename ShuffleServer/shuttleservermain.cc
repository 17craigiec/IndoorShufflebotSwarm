#include "ShuffleServer.hh"

// Global Inputs
std::string g_interface_name = "wlo1";
int g_server_port = 7600;
int g_verbose_level = 0;
int g_cycle_count = 0;
bool g_prog_kill = false;

// Helper defintions
int parseArgs(int argc, char const *argv[]);
void SignalHandler(int sig);


int main(int argc, char const *argv[])
{
    if(parseArgs(argc, argv))
        return 1;

    // Register signal handler
    signal(SIGINT, SignalHandler);

    ShuffleServer server(g_interface_name, g_server_port, g_verbose_level);
    if(server.Start() != 0)
    {
        std::cerr << "Failed to start server" << std::endl;
        return -1;
    }

    std::cout << "Listening for UDP packets on port " << g_server_port << "..." << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    // Keep running until interrupted
    while(server.IsRunning() && !g_prog_kill)
    {
        g_cycle_count++;
        
        std::cout << "Cycle #: " << g_cycle_count << std::endl;

        // Every 5th cycle send the timesync message to all clients
        if (g_cycle_count % 5 == 0)
        {
            server.SendTimesyncToClients();
        }

        sleep(1);
    }

    server.Stop();
    std::cout << "Shutdown complete." << std::endl;
    return 0;
}


// Helper functions
int parseArgs(int argc, char const *argv[])
{
    // If help is requested
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            printf("\n[HELP] ShuffleServer - A server to manage shufflebot clients.\n");
            printf("=============================================================\n");
            printf("\nUsage: %s -i <interface_name> -p <server_port> -v <verbose_level>\n", argv[0]);
            printf("EX: %s -i %s -p %d -v %d\n\n", argv[0], g_interface_name.c_str(), g_server_port, g_verbose_level);
            PrintAvailableInterfaces();
            printf("\n");
            return 1;
        }

        if (arg == "-i")
        {
            if (i + 1 < argc)
            {
                g_interface_name = argv[i + 1];
                i++;
            }
            else
            {
                printf("[Error] Interface name not provided\n");
                return 1;
            }
        }

        else if (arg == "-p")
        {
            if (i + 1 < argc)
            {
                g_server_port = std::stoi(argv[i + 1]);
                i++;
            }
            else
            {
                printf("[Error] Server port not provided\n");
                return 1;
            }
        }

        else if (arg == "-v")
        {
            if (i + 1 < argc)
            {
                g_verbose_level = std::stoi(argv[i + 1]);
                i++;
                printf("[Info] Verbose level set to %d\n", g_verbose_level);
            }
            else
            {
                printf("[Error] Verbose level not provided\n");
                return 1;
            }
        }
    }

    return 0;
}

void SignalHandler(int sig)
{
    std::cout << "\nReceived Ctrl+C, shutting down..." << std::endl;
    g_prog_kill = true;
}
