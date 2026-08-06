#pragma once

#include <shm_types.hpp>

#include <advrf_middleware_core/shared_memory/shm_bridge_connection.hpp>
#include "advrf_middleware_core/utils/channel.hpp"

class PublisherShmConnection
    : public ShmMiddlewareBridgeConnection<PublisherShmConnection, SharedProtoPubBridge>
{
public:
    using Payload = decltype(SharedProtoPubBridge::payload);
    using Queue = Payload::Queue;

    bool remote_ready() const
    {
        return bridge().status.shm_ready.load(std::memory_order_acquire);
    }

    bool connect(const std::string& shm_name)
    {
        return ShmMiddlewareBridgeConnection<PublisherShmConnection, SharedProtoPubBridge>::connect(shm_name);
    }

    Queue& resolve(Channel channel)
    {
        switch (channel)
        {
            case Channel::Imu:
                return bridge().payload.queue_for(DeviceType::IMU);

            case Channel::Motor:
                return bridge().payload.queue_for(DeviceType::MOTOR);

            case Channel::Gripper:
                return bridge().payload.queue_for(DeviceType::GRIPPER);

            case Channel::Pump:
                return bridge().payload.queue_for(DeviceType::PUMP);

            case Channel::PowerBoard:
                return bridge().payload.queue_for(DeviceType::POWER_BOARD);

            case Channel::ForceTorque:
                return bridge().payload.queue_for(DeviceType::FORCE_TORQUE);

            case Channel::Valve:
                return bridge().payload.queue_for(DeviceType::VALVE);

            default:
                throw std::out_of_range("Invalid channel");
        }
    }

private:
};