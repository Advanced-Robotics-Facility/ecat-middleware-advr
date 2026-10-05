#pragma once

#include <cstddef>
#include <map>
#include <array>
#include <optional>

// from ecat_master
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
    PowerBoard,
    ForceTorque,
    Count
};

/// Mapping from middleware receive channels to EtherCAT shared-memory devices.
const std::map<ChannelRx, DeviceTypeRx> k_map_channelrx_to_devicerx = {
    {ChannelRx::Imu, DeviceTypeRx::IMU},
    {ChannelRx::Motor, DeviceTypeRx::MOTOR},
    {ChannelRx::Gripper, DeviceTypeRx::GRIPPER},
    {ChannelRx::Pump, DeviceTypeRx::PUMP},
    {ChannelRx::PowerBoard, DeviceTypeRx::POWER_BOARD},
    {ChannelRx::ForceTorque, DeviceTypeRx::FORCE_TORQUE},
    {ChannelRx::Valve, DeviceTypeRx::VALVE}
};

/// Mapping from middleware transmit channels to EtherCAT shared-memory devices.
const std::map<ChannelTx, DeviceTypeTx> k_map_channeltx_to_devicetx = {
    {ChannelTx::Motor, DeviceTypeTx::MOTOR},
    {ChannelTx::Gripper, DeviceTypeTx::GRIPPER},
    {ChannelTx::Pump, DeviceTypeTx::PUMP},
    {ChannelTx::Valve, DeviceTypeTx::VALVE},
    {ChannelTx::PowerBoard, DeviceTypeTx::POWERBOARD},
    {ChannelTx::ForceTorque, DeviceTypeTx::FORCETORQUE},
};

/**
 * @brief Return the shared-memory device associated with a receive channel.
 *
 * @return The device, or @c std::nullopt for an invalid/unmapped channel.
 */
inline std::optional<DeviceTypeRx>
device_for(ChannelRx channel)
{
    const auto it = k_map_channelrx_to_devicerx.find(channel);

    if (it == k_map_channelrx_to_devicerx.end())
        return std::nullopt;

    return it->second;
}

/**
 * @brief Return the shared-memory device associated with a transmit channel.
 *
 * @return The device, or @c std::nullopt for an invalid/unmapped channel.
 */
inline std::optional<DeviceTypeTx>
device_for(ChannelTx channel)
{
    const auto it = k_map_channeltx_to_devicetx.find(channel);

    if (it == k_map_channeltx_to_devicetx.end())
        return std::nullopt;

    return it->second;
}

/// Number of valid receive channels.
static constexpr std::size_t CHANNEL_COUNT = static_cast<std::size_t>(ChannelRx::Count);

/// All valid receive channels, in enum order.
inline static constexpr std::array<ChannelRx, CHANNEL_COUNT> CHANNELS_ARRAY{
      ChannelRx::Imu,  ChannelRx::Motor,      ChannelRx::Gripper,
      ChannelRx::Pump, ChannelRx::PowerBoard, ChannelRx::ForceTorque,
      ChannelRx::Valve};

}