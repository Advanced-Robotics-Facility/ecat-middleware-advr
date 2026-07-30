#pragma once

#include <array>

// ecat_master_future
#include <ecat_master_future/shm/bridge_struct.hpp>
#include <advrf_middleware_core/shared_memory/shm_bridge_connection.hpp>

// advrf_middleware_core
#include "advrf_middleware_core/utils/channel.hpp"

class PublisherShmConnection
    : public ShmMiddlewareBridgeConnection<PublisherShmConnection, SharedPubBridge>
{
public:
    using Queue = decltype(SharedPubBridge::payload.imu);
    bool remote_ready() const
    {
        return bridge().status.rt_ready.load();
    }

    void set_ready(bool ready)
    {
        bridge().status.mw_ready.store(ready);
    }

    bool connect(const std::string& shm_name)
    {
        if (!ShmMiddlewareBridgeConnection<PublisherShmConnection, SharedPubBridge>::connect(shm_name))
            return false;

        channels_ = {
            &bridge().payload.imu,
            &bridge().payload.motor,
            &bridge().payload.gripper,
            &bridge().payload.pump,
            &bridge().payload.power_board,
            &bridge().payload.force_torque,
            &bridge().payload.valve
        };

        return true;
    }

    Queue& resolve(Channel channel)
    {
        return *channels_[static_cast<std::size_t>(channel)];
    }

private:
    std::array<Queue*, static_cast<std::size_t>(Channel::Count)> channels_{};
};