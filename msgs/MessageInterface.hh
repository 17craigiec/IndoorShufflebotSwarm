#pragma once

#include "SpeedCurvCommand.hh"
#include "Telemetry.hh"
#include "TimeSync.hh"

#include <cstddef>
#include <cstring>
#include <cstdio>

#define MAX_MSG_SIZE 256

enum MessageType
{
    MTYPE_UNKNOWN = 0,
    MTYPE_TELEMETRY = 1,
    MTYPE_SPEED_CURV_COMMAND = 2,
    MTYPE_TIME_SYNC = 3
};

class MessageInterface
{
private:
    /* data */
public:
    MessageInterface(/* args */);
    ~MessageInterface();

    // Read Members
    SpeedCurvCommand speed_curv_cmd_;
    Telemetry telemetry_;
    TimeSync time_sync_;

    MessageType last_msg_type_;
    float last_msg_mono_sec_;

    // Write Members
    unsigned char write_buffer_[MAX_MSG_SIZE];

    // Reads raw data and populates the appropriate message struct based on the first byte (message type).
    // Returns 0 on success, -1 on failure (e.g. invalid message)
    int ParseMessage(const unsigned char *_data, MessageType &_type);

    // Writes into the provided buffer, prefixed with the message type byte.
    // Returns 0 on success, -1 on failure (e.g. invalid message)
    int WriteSpeedCurvCommand(const SpeedCurvCommand &_cmd);
    int WriteTelemetry(const Telemetry &_telemetry);
    int WriteTimeSync(const TimeSync &_time_sync);
};

MessageInterface::MessageInterface(/* args */)
{
};

MessageInterface::~MessageInterface()
{
};

int MessageInterface::ParseMessage(const unsigned char *_data, MessageType &_type)
{
    _type = static_cast<MessageType>(_data[0]);

    switch (_type)
    {
    case MTYPE_TELEMETRY:
        memcpy(&telemetry_, _data + 1, sizeof(Telemetry));
        break;
    case MTYPE_SPEED_CURV_COMMAND:
        memcpy(&speed_curv_cmd_, _data + 1, sizeof(SpeedCurvCommand));
        break;
    case MTYPE_TIME_SYNC:
        memcpy(&time_sync_, _data + 1, sizeof(TimeSync));
        break;
    default:
        return -1; // Unknown message type
    }

    last_msg_type_ = _type;
    return 0; // Success
};


int MessageInterface::WriteSpeedCurvCommand(const SpeedCurvCommand &_cmd)
{
    size_t needed = 1 + sizeof(SpeedCurvCommand);
    if (needed > MAX_MSG_SIZE)
    {
        printf("Needed: %zu, Max: %d\n", needed, MAX_MSG_SIZE);
        return -1;  // Destination too small
    }

    write_buffer_[0] = static_cast<unsigned char>(MTYPE_SPEED_CURV_COMMAND);
    memcpy(write_buffer_ + 1, &_cmd, sizeof(SpeedCurvCommand));
    return 0; // Success
}

int MessageInterface::WriteTelemetry(const Telemetry &_telemetry)
{
    size_t needed = 1 + sizeof(Telemetry);
    if (needed > MAX_MSG_SIZE)
    {
        printf("Needed: %zu, Max: %d\n", needed, MAX_MSG_SIZE);
        return -1;  // Destination too small
    }

    write_buffer_[0] = static_cast<unsigned char>(MTYPE_TELEMETRY);
    memcpy(write_buffer_ + 1, &_telemetry, sizeof(Telemetry));
    return 0; // Success
}

int MessageInterface::WriteTimeSync(const TimeSync &_time_sync)
{
    size_t needed = 1 + sizeof(TimeSync);
    if (needed > MAX_MSG_SIZE)
    {
        printf("Needed: %zu, Max: %d\n", needed, MAX_MSG_SIZE);
        return -1;  // Destination too small
    }

    write_buffer_[0] = static_cast<unsigned char>(MTYPE_TIME_SYNC);
    memcpy(write_buffer_ + 1, &_time_sync, sizeof(TimeSync));
    return 0; // Success
}