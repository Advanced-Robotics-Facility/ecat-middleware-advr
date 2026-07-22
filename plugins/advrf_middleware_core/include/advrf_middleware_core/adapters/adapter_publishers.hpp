#pragma once

#include "advrf_middleware_core/robot_config.hpp"
#include "advrf_middleware_core/utils/log.hpp"
#include "advrf_middleware_core/pdo_utils.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <ecat_master_future/shm_shared_types.hpp>

#include <ecat_master_future/shm_utils.hpp>



inline std::unique_ptr<SharedMemoryClient> wait_for_shared_memory()
{
    while (true) {
        auto shm = std::make_unique<SharedMemoryClient>(SHM_NAME, sizeof(SharedPubBridge));
        LOG_DEBUG("wait connection");
        if (shm->is_valid())
            return shm;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return nullptr;
}

namespace middleware_adapter::message {

class AdapterPublishers
{
public:

    using Pdo = iit::advrf::Ec_slave_pdo;
    using Queue = decltype(SharedPubBridge::imu);

    enum class Channel : std::size_t
    {
        Imu,
        Motor,
        Gripper,
        Pump,
        PowerBoard,
        Count
    };

    class ICallback
    {
    public:
        virtual ~ICallback() = default;

        virtual void on_entry() = 0;
        virtual void on_pdo(const Pdo& pdo) = 0;
        virtual void on_exit() = 0;
    };

    struct Subscription
    {
        ICallback* callback;
        std::vector<Channel> channels;
    };

    AdapterPublishers() = default;
    AdapterPublishers(SharedPubBridge* bridge)
        : bridge_(bridge){}
    virtual ~AdapterPublishers() = default;

    virtual bool init(const RobotConfig& cfg) = 0;

    bool shm_connect()
    {
        shm_ = wait_for_shared_memory();
         if (!shm_) return false;

        bridge_.reset(shm_->get<SharedPubBridge>());
        while (!bridge_->mw_ready.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        channel_map_ = {
            {Channel::Imu,        &bridge_->imu},
            {Channel::Motor,      &bridge_->motor},
            {Channel::Gripper,    &bridge_->gripper},
            {Channel::Pump,       &bridge_->pump},
            {Channel::PowerBoard, &bridge_->power_board},
        };

        return true;
    }

    void shm_declare_ready()
    {
        if(bridge_ == nullptr) {
            LOG_ERROR("Bridge not setup");
            return;
        }
        bridge_->rt_ready.store(true);
    }

    void shm_declare_not_ready()
    {
        if(bridge_ == nullptr) {
            LOG_ERROR("Bridge not setup");
            return;
        }
        bridge_->rt_ready.store(false);
    }

    bool is_connected() const
    {
        return bridge_ != nullptr;
    }

    bool is_ok() const
    {
        return bridge_ != nullptr && bridge_->mw_ready.load();
    }

    void spin_once()
    {
        for (auto& sub : subscriptions_)
        {
            sub.callback->on_entry();

            for (auto channel : sub.channels)
            {
                auto it = channel_map_.find(channel);
                if (it != channel_map_.end())
                    process_one_queue(*it->second, sub.callback);
            }

            sub.callback->on_exit();
        }
    }

protected:
    void subscribe(Subscription subscription)
    {
        subscriptions_.push_back(subscription);
    }

    void subscribe(ICallback* subscription, std::vector<Channel> channels)
    {
        subscriptions_.push_back({subscription, std::move(channels)});
    }

    std::unique_ptr<SharedPubBridge> bridge_ = nullptr;
    std::unique_ptr<SharedMemoryClient> shm_ = nullptr;

private:

    template<typename Queue>
    void process_one_queue(Queue& queue, ICallback* callback)
    {
        ProtoSlot frame;
        while (queue.try_pop(frame)) {
            Pdo pdo;
            if (!pdo_utils::parse_frame(
                    frame.data,
                    static_cast<ssize_t>(frame.size),
                    pdo))
            {
                continue;
            }
            callback->on_pdo(pdo);
        }
    }

    template<typename... Queues>
    void process_queues(ICallback* callback, Queues&... queues)
    {
        if (!bridge_) {
            LOG_ERROR("Bridge not setup");
            return;
        }

        if (!callback) {
            LOG_ERROR("Callback not setup");
            return;
        }

        callback->on_entry();
        (process_one_queue(queues, callback), ...);
        callback->on_exit();
}

    std::array<Queue*, static_cast<size_t>(Channel::Count)> channels_;
    std::unordered_map<Channel, Queue*> channel_map_;
    std::vector<Subscription> subscriptions_;

    ShmProtoHelper proto_helper_;
};

}