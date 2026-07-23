#pragma once

#include <array>

#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>

#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/shared_memory/shm_bridge_connection.hpp"

class PublisherShmConnection
    : public ShmBridgeConnection<PublisherShmConnection, SharedPubBridge>
{
public:
    using Queue = decltype(SharedPubBridge::imu);

    bool peer_ready() const
    {
        return bridge_->mw_ready.load();
    }

    void set_local_ready(bool ready)
    {
        bridge_->rt_ready.store(ready);
    }

    bool connect(const std::string& shm_name)
    {
        if (!ShmBridgeConnection::connect(shm_name))
            return false;

        channels_ = {
            &bridge_->imu,
            &bridge_->motor,
            &bridge_->gripper,
            &bridge_->pump,
            &bridge_->power_board,
            &bridge_->force_torque
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