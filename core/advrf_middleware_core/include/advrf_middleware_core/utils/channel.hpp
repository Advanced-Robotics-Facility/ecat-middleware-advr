#pragma once

#include <cstddef>

enum class Channel : std::size_t
    {
        Imu,
        Motor,
        Gripper,
        Pump,
        PowerBoard,
        ForceTorque,
        Count
    };