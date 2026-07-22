#pragma once

#include "advrf_middleware_core/robot_config.hpp"
#include "advrf_middleware_core/utils/log.hpp"
#include "advrf_middleware_core/pdo_utils.hpp"
#include "advrf_middleware_core/shared_memory/shm_connection_publishers.hpp"
#include "advrf_middleware_core/utils/channel.hpp"

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
    using Queue = decltype(SharedPubBridge::imu); // supposed all queues have the same type, which is true for now

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
    virtual ~AdapterPublishers() = default;

    virtual bool init(const RobotConfig& cfg) = 0;

    PublisherShmConnection& shm()
    {
        return shm_;
    }

    const PublisherShmConnection& shm() const
    {
        return shm_;
    }

    void spin_once()
    {
        for (auto& sub : subscriptions_)
        {
            sub.callback->on_entry();

            for (auto channel : sub.channels)
            {
                process_one_queue(
                    shm_.resolve(channel),
                    sub.callback);
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

    PublisherShmConnection shm_;

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
        if (!callback) {
            LOG_ERROR("Callback not setup");
            return;
        }

        callback->on_entry();
        (process_one_queue(queues, callback), ...);
        callback->on_exit();
}

    std::vector<Subscription> subscriptions_;

    ShmProtoHelper proto_helper_;
};

}