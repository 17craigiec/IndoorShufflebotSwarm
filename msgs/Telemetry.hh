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
    double uptime_sec_;

    Cartesian pos_;
    Cartesian vel_;
    Cartesian acc_;
    float heading_;
};

inline void printTelemetry(const Telemetry &_t)
{
    // Print all fields of the parsed telemetry message
    printf("Telemetry:\n");
    printf("  Robot ID: %d\n", static_cast<int>(_t.robot_id_));
    printf("  Uptime: %f sec\n", _t.uptime_sec_);
    printf("  Position: (%f, %f)\n", _t.pos_.x_, _t.pos_.y_);
    printf("  Velocity: (%f, %f)\n", _t.vel_.x_, _t.vel_.y_);
    printf("  Acceleration: (%f, %f)\n", _t.acc_.x_, _t.acc_.y_);
    printf("  Heading: %f degrees\n", _t.heading_);
};