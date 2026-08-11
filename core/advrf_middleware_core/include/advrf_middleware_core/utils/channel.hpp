#pragma once

#include <cstddef>
#include <map>
#include <optional>

// from ecat_master
#include <shm_types.hpp>

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

enum class ChannelTx : std::size_t
{
    Motor,
    Pump,
    Valve,
    Gripper,
    Count
};

const std::map<ChannelRx, DeviceTypeRx> k_map_channelrx_to_devicerx = {
    {ChannelRx::Imu, DeviceTypeRx::IMU},
    {ChannelRx::Motor, DeviceTypeRx::MOTOR},
    {ChannelRx::Gripper, DeviceTypeRx::GRIPPER},
    {ChannelRx::Pump, DeviceTypeRx::PUMP},
    {ChannelRx::PowerBoard, DeviceTypeRx::POWER_BOARD},
    {ChannelRx::ForceTorque, DeviceTypeRx::FORCE_TORQUE},
    {ChannelRx::Valve, DeviceTypeRx::VALVE}
};

const std::map<ChannelTx, DeviceTypeTx> k_map_channeltx_to_devicetx = {
    {ChannelTx::Motor, DeviceTypeTx::MOTOR},
    {ChannelTx::Gripper, DeviceTypeTx::GRIPPER},
    {ChannelTx::Pump, DeviceTypeTx::PUMP},
    {ChannelTx::Valve, DeviceTypeTx::VALVE}
};


inline std::optional<DeviceTypeRx>
device_for(ChannelRx channel)
{
    const auto it = k_map_channelrx_to_devicerx.find(channel);

    if (it == k_map_channelrx_to_devicerx.end())
        return std::nullopt;

    return it->second;
}

inline std::optional<DeviceTypeTx>
device_for(ChannelTx channel)
{
    const auto it = k_map_channeltx_to_devicetx.find(channel);

    if (it == k_map_channeltx_to_devicetx.end())
        return std::nullopt;

    return it->second;
}