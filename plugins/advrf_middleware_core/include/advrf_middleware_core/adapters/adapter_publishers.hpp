#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/robot_config.hpp"
#include "advrf_middleware_core/utils/log.hpp"
#include "advrf_middleware_core/pdo_utils.hpp"
#include "advrf_middleware_core/shared_memory/shm_connection_publishers.hpp"
#include "advrf_middleware_core/utils/channel.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>

inline int get_ecat_id(const std::string& component_name)
{
    size_t pos = component_name.rfind('_');
    if (pos == std::string::npos || pos + 1 >= component_name.size()){
        LOG_ERROR("Format ecat name not correct, {}", component_name);
        return -1;
    }

    int out = -1;
    try{
         out = std::stoi(component_name.substr(pos + 1));
    }
    catch (...){
        LOG_ERROR("Failed to convert ecat id from string, {}", component_name.substr(pos + 1));
        return -1;
    }
    return out;
}

namespace middleware_adapter::message  {

class AdapterPublishers : public AdapterBase
{
public:

    using Pdo = iit::advrf::Ec_slave_pdo;

    struct CachedPdo
    {
        uint32_t ecat_id;
        Pdo pdo;
    };
    using Cache = std::unordered_map<Channel, std::vector<CachedPdo>>;
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
        std::vector<uint32_t> ids_allowed;

        private:
            friend class AdapterPublishers;
            std::unordered_set<uint32_t> ids_allowed_set;
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

    void spin_once() override
    {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        cache_.clear();
        fill_cache(cache_);
        for (auto& sub : subscriptions_)
        {
            std::unordered_set<uint32_t> ids_seen;
            sub.callback->on_entry();
            for (auto channel : sub.channels)
            {
                auto it = cache_.find(channel);
                if (it != cache_.end())
                {
                    process_cache(
                        it->second,
                        sub.callback,
                        sub.ids_allowed_set,
                        ids_seen);
                }
            }

            if (ids_seen == sub.ids_allowed_set || true)
            {
                sub.callback->on_exit();
            }  
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - now;
        // LOG_DEBUG("AdapterPublishers spin_once elapsed time: {} ms", elapsed.count());
    }

protected:
    void subscribe(Subscription subscription)
    {
        for (auto id : subscription.ids_allowed)
        {
            if (!subscription.ids_allowed_set.insert(id).second)
            {
                LOG_ERROR("Duplicate configured ECAT ID {}", id);
            }
        }

        subscriptions_.push_back(std::move(subscription));
    }

    void subscribe(ICallback* subscription,
                std::vector<Channel> channels,
                std::vector<uint32_t> ids_allowed = {})
    {
        Subscription sub;
        sub.callback = subscription;
        sub.channels = std::move(channels);
        sub.ids_allowed = std::move(ids_allowed);

        for (auto id : sub.ids_allowed)
        {
            if (!sub.ids_allowed_set.insert(id).second)
            {
                LOG_ERROR("Duplicate configured ECAT ID {}", id);
            }
        }

        subscriptions_.push_back(std::move(sub));
    }

    template<typename CallbackObject>
    CallbackObject& register_callback(std::vector<Channel> channels, std::vector<uint32_t> ids_allowed = {})
    {
        auto pub = std::make_shared<CallbackObject>();
        subscribe(pub.get(), std::move(channels), std::move(ids_allowed));
        callbacks_.push_back(pub);
        return *(pub);
    }

    PublisherShmConnection shm_;

private:
    std::vector<std::shared_ptr<ICallback>> callbacks_;

    void fill_cache(Cache& cache)
    {
        for (auto& [_, v] : cache_)
            v.clear();
        for (auto channel : all_channels_)
        {
            auto& queue = shm_.resolve(channel);
            ProtoSlot frame;
            while (queue.try_pop(frame))
            {
                Pdo pdo;
                if (!pdo_utils::parse_frame(
                        frame.data,
                        static_cast<ssize_t>(frame.size),
                        pdo))
                {
                    continue;
                }

                int ecat_id = get_ecat_id(pdo.header().str_id());
                if (ecat_id < 0)
                {
                    LOG_ERROR("Format error for PDO frame with ID {}", pdo.header().str_id());
                    continue;
                }

                cache[channel].push_back(
                {
                    static_cast<uint32_t>(ecat_id),
                    std::move(pdo)
                });
            }
        }
    }

    void process_cache(
        const std::vector<CachedPdo>& pdos,
        ICallback* callback,
        const std::unordered_set<uint32_t>& ids_allowed,
        std::unordered_set<uint32_t>& ids_seen)
    {
        for (const auto& frame : pdos)
        {
            const auto id = frame.ecat_id;

            if (!ids_allowed.empty() &&
                ids_allowed.find(id) == ids_allowed.end())
            {
                continue;
            }

            if (!ids_seen.insert(id).second)
            {
                //LOG_ERROR("Duplicate PDO frame for ECAT ID {}", id);
                continue;
            }

            callback->on_pdo(frame.pdo);
        }
    }

    std::unordered_set<Channel> all_channels_ = {
        Channel::Imu,
        Channel::Motor,
        Channel::Gripper,
        Channel::Pump,
        Channel::PowerBoard,
        Channel::ForceTorque
    };
    std::vector<Subscription> subscriptions_;
    ShmProtoHelper proto_helper_;
    Cache cache_;
};

}