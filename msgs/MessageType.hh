#pragma once

#define MAX_MSG_SIZE 256

enum MessageType
{
    MTYPE_UNKNOWN = 0,
    MTYPE_TELEMETRY = 1,
    MTYPE_SPEED_CURV_COMMAND = 2,
    MTYPE_TIME_SYNC = 3,

    MTYPE_MAX = 4 // Not a valid message type, used for bounds checking
};

inline bool isValidMessageType(int value) {
    return value >= MTYPE_UNKNOWN && value < MTYPE_MAX;
}