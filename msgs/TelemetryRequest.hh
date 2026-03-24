#pragma once

#include <cstdint>

struct TelemetryRequest
{
    // Sending ID zero requests telemetry from all robots.
    // Setting a specific ID will target a client for an update
    uint8_t robot_id_ = 0;

    // Increments as the server sends requests
    unsigned int serial_number_ = 0;
};