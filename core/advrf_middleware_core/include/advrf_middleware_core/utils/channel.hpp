#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <optional>

#include <shm_types.hpp>
#include <esc_type.h>

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
      
inline ChannelRx get_channel(uint32_t esc_type)
{
    const auto it = iit::ecat::esc_type_map.find(esc_type);
    if (it == iit::ecat::esc_type_map.end()) {
        throw std::invalid_argument("Unknown ESC type");
    }

    const std::string& name = it->second;

    if (name.find("Motor") != std::string::npos)    return ChannelRx::Motor;
    if (name.find("Imu") != std::string::npos)      return ChannelRx::Imu;
    if (name.find("Gripper") != std::string::npos)  return ChannelRx::Gripper;
    if (name.find("Hpu") != std::string::npos)      return ChannelRx::Pump;
    if (name.find("Pow") != std::string::npos)      return ChannelRx::PowerBoard;
    if (name.find("Ft") != std::string::npos)       return ChannelRx::ForceTorque;
    if (name.find("Valve") != std::string::npos)    return ChannelRx::Valve;

    throw std::invalid_argument("ESC type has no receive channel: " + name);
}

}