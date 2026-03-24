#include "MessageType.hh"
#include "MessageSerializer.hh"
#include "Telemetry.hh"

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

    // Example usage of MessageInterface with random telemetry data
    Telemetry *telemetry = new Telemetry();
    telemetry->robot_id_ = instance_id;
    telemetry->pos_.x_ = 1.0f * instance_id;
    telemetry->pos_.y_ = 2.0f * instance_id;
    telemetry->vel_.x_ = 0.5f * instance_id;
    telemetry->vel_.y_ = 0.5f * instance_id;
    telemetry->acc_.x_ = 0.1f * instance_id;
    telemetry->acc_.y_ = 0.1f * instance_id;
    telemetry->heading_ = 45.0f * instance_id;

    printf("\nStaring Telemetry - To be written.\n");
    printTelemetry(*telemetry);

    // Initialize WRITE message interface
    // =============================================================================
    MessageSerializer<Telemetry> telemetry_writer(telemetry, MTYPE_TELEMETRY);
    // TelemetrySerializer telemetry_writer(telemetry);
    unsigned int msg_size = telemetry_writer.GetMessageSize();
    unsigned char *write_buffer = new unsigned char[msg_size];

    if (telemetry_writer.Write(write_buffer, msg_size) == 0)
    {
        std::cout << "Telemetry message written successfully!" << std::endl;
    }
    else
    {
        std::cerr << "Failed to write telemetry message." << std::endl;
    }

    // Initialize READ message interface
    // =============================================================================
    Telemetry output_telemetry;
    MessageSerializer<Telemetry> telemetry_reader(MTYPE_TELEMETRY);
    // Take the write buffer and parse it back to verify correctness
    if (telemetry_reader.Read(write_buffer, msg_size, &output_telemetry) == 0)
    {
        std::cout << "Message parsed successfully!" << std::endl;
    }
    else
    {
        std::cerr << "Failed to parse message." << std::endl;
    }

    printf("\nParsed Telemetry - Has been read.\n");
    printTelemetry(output_telemetry);

    return 0;
}
