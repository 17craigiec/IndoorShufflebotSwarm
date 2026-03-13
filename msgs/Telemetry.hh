#pragma once

#include <cstdint>

struct Cartesian
{
    float x_;
    float y_;
};

struct Telemetry
{
    uint8_t robot_id_;

    Cartesian pos_;
    Cartesian vel_;
    Cartesian acc_;

    float heading_;
};