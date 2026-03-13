#include "MessageInterface.hh"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char const *argv[])
{
    int instance_id = 0;

    // Parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-i" && i + 1 < argc)
        {
            instance_id = std::atoi(argv[++i]);
        }

        if (arg == "-h" || arg == "--help")
        {
            std::cout << "Usage: " << argv[0] << " -i <instance_id>" << std::endl;
            return 0;
        }
    }
    std::cout << "Instance ID: " << instance_id << std::endl;

    // Initialize message interface
    MessageInterface msg_interface;

    // Example usage of MessageInterface
    Telemetry telemetry;
    telemetry.robot_id_ = instance_id;
    telemetry.pos_.x_ = 1.0f * instance_id;
    telemetry.pos_.y_ = 2.0f * instance_id;
    telemetry.vel_.x_ = 0.5f * instance_id;
    telemetry.vel_.y_ = 0.5f * instance_id;
    telemetry.acc_.x_ = 0.1f * instance_id;
    telemetry.acc_.y_ = 0.1f * instance_id;
    telemetry.heading_ = 45.0f * instance_id;

    if (msg_interface.WriteTelemetry(telemetry) == 0)
    {
        std::cout << "Telemetry message written successfully!" << std::endl;
    }
    else
    {
        std::cerr << "Failed to write telemetry message." << std::endl;
    }

    // Take the write buffer and parse it back to verify correctness
    MessageType parsed_type;
    if (msg_interface.ParseMessage(msg_interface.write_buffer_, parsed_type) == 0)
    {
        std::cout << "Message parsed successfully! Type: " << parsed_type << std::endl;
    }
    else
    {
        std::cerr << "Failed to parse message." << std::endl;
    }

    // Print all fields of the parsed telemetry message
    if (parsed_type == MTYPE_TELEMETRY)
    {
        std::cout << "Parsed Telemetry:" << std::endl;
        std::cout << "  Robot ID: " << msg_interface.telemetry_.robot_id_ << std::endl;
        std::cout << "  Position: (" << msg_interface.telemetry_.pos_.x_ << ", " << msg_interface.telemetry_.pos_.y_ << ")" << std::endl;
        std::cout << "  Velocity: (" << msg_interface.telemetry_.vel_.x_ << ", " << msg_interface.telemetry_.vel_.y_ << ")" << std::endl;
        std::cout << "  Acceleration: (" << msg_interface.telemetry_.acc_.x_ << ", " << msg_interface.telemetry_.acc_.y_ << ")" << std::endl;
        std::cout << "  Heading: " << msg_interface.telemetry_.heading_ << std::endl;
    }

    return 0;
}
