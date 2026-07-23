#pragma once

#include <array>
#include <memory>
#include <thread>

#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>

#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/utils/log.hpp"

class PublisherShmConnection
{
public:
    using Queue = decltype(SharedPubBridge::imu);

    bool connect(const std::string& shm_name)
    {
        while (true)
        {
            shm_ = std::make_unique<SharedMemoryClient>(
                shm_name.c_str(),
                sizeof(SharedPubBridge));

            if (shm_->is_valid())
                break;
            LOG_DEBUG("Wait connection shared memory, {}", shm_name);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        bridge_ = shm_->get<SharedPubBridge>();
        while (!bridge_->mw_ready.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        channels_ = {
            &bridge_->imu,
            &bridge_->motor,
            &bridge_->gripper,
            &bridge_->pump,
            &bridge_->power_board
        };

        return true;
    }

    void declare_ready()
    {
        if (bridge_)
            bridge_->rt_ready.store(true);
    }

    void declare_not_ready()
    {
        if (bridge_)
            bridge_->rt_ready.store(false);
    }

    bool is_connected() const
    {
        return bridge_ != nullptr;
    }

    bool is_ok() const
    {
        return bridge_ && bridge_->mw_ready.load();
    }

    Queue& resolve(Channel channel)
    {
        return *channels_[static_cast<std::size_t>(channel)];
    }

    const Queue& resolve(Channel channel) const
    {
        return *channels_[static_cast<std::size_t>(channel)];
    }

    SharedPubBridge& bridge()
    {
        return *bridge_;
    }

    const SharedPubBridge& bridge() const
    {
        return *bridge_;
    }

    void close()
    {
        declare_not_ready();
        bridge_ = nullptr;
        shm_.reset();
    }

private:
    std::unique_ptr<SharedMemoryClient> shm_;
    SharedPubBridge* bridge_ = nullptr;

    std::array<Queue*, static_cast<std::size_t>(Channel::Count)> channels_{};
};