#pragma once

#include <array>

enum class Channel : std::size_t
    {
        Imu,
        Motor,
        Gripper,
        Pump,
        PowerBoard,
        Count
    };