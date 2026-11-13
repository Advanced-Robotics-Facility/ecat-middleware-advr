#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <optional>

#include <shm_types.hpp>

namespace advrf::middleware::shm {

/**
 * @brief Receive data channels accepted by the middleware.
 *
 * @c Count is a sentinel and not a valid channel.
 */
enum class ChannelRx : std::size_t
{
    Imu,
    Motor,
    Gripper,
    Pump,
    PowerBoard,
    ForceTorque,
    Valve,
    Count
};

/**
 * @brief Transmit command channels exposed by the middleware.
 *
 * @c Count is a sentinel and not a valid channel.
 */
enum class ChannelTx : std::size_t
{
    Motor,
    Pump,
    Valve,
    Gripper,
    Count
};

/// Number of valid receive channels.
static constexpr std::size_t CHANNEL_COUNT = static_cast<std::size_t>(ChannelRx::Count);

/// All valid receive channels, in enum order.
inline static constexpr std::array<ChannelRx, CHANNEL_COUNT> CHANNELS_ARRAY{
      ChannelRx::Imu,  ChannelRx::Motor,      ChannelRx::Gripper,
      ChannelRx::Pump, ChannelRx::PowerBoard, ChannelRx::ForceTorque,
      ChannelRx::Valve};

}