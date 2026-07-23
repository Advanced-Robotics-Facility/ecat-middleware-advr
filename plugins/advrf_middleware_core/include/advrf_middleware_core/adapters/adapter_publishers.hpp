#pragma once

#include <string>

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
        std::vector<uint32_t> ids_allowed;
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
                process_one_queue(shm_.resolve(channel),sub.callback, sub.ids_allowed);
            }
            sub.callback->on_exit();
        }
    }

protected:
    void subscribe(Subscription subscription)
    {
        subscriptions_.push_back(subscription);
    }

    void subscribe(ICallback* subscription, std::vector<Channel> channels, std::vector<uint32_t> ids_allowed = {})
    {
        subscriptions_.push_back({subscription, 
                                    std::move(channels), 
                                    std::move(ids_allowed)});
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

    template<typename Queue>
    void process_one_queue(Queue& queue, ICallback* callback, const std::vector<uint32_t>& ids_allowed = {})
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

            if(ids_allowed.size() > 0) {
                int ecat_id = get_ecat_id(pdo.header().str_id());
                if (ecat_id < 0) {
                    LOG_ERROR("Format error for PDO frame with ID: {}", pdo.header().str_id());
                    continue;
                }
                if (std::find(ids_allowed.begin(), ids_allowed.end(), static_cast<uint32_t>(ecat_id)) == ids_allowed.end()) {
                    //LOG_DEBUG("Skipping PDO frame with ID: {} (not in allowed list)", pdo.header().str_id());
                    continue;
                }
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