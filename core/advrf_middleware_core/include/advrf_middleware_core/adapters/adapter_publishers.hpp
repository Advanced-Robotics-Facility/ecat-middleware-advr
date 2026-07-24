#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/pdo_utils.hpp"
#include "advrf_middleware_core/shared_memory/shm_connection_publishers.hpp"
#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/utils/log.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>

inline int get_ecat_id(const std::string& component_name)
{
    const std::size_t pos = component_name.rfind('_');

    if (pos == std::string::npos || pos + 1 >= component_name.size()) {
        LOG_ERROR("Format ecat name not correct, {}", component_name);
        return -1;
    }

    try {
        return std::stoi(component_name.substr(pos + 1));
    }
    catch (...) {
        LOG_ERROR(
            "Failed to convert ecat id from string, {}",
            component_name.substr(pos + 1));
        return -1;
    }
}

namespace middleware_adapter::message {

class AdapterPublishers : public AdapterBase
{
public:
    using Pdo = iit::advrf::Ec_slave_pdo;
    using EcatId = std::uint32_t;

    static constexpr std::size_t MaxEcatIds = 256;

    struct CachedPdo
    {
        EcatId ecat_id = 0;
        Pdo pdo;
    };

    using Cache = std::unordered_map<Channel, std::vector<CachedPdo>>;
    using IdMask = std::bitset<MaxEcatIds>;
    using Queue = decltype(SharedPubBridge::imu);

    class IPublisher
    {
    public:
        virtual ~IPublisher() = default;

        virtual void begin_cycle() = 0;
        virtual void consume(const Pdo& pdo) = 0;
        virtual void end_cycle(bool valid) = 0;
    };

    struct Subscription
    {
        IPublisher* publisher = nullptr;
        std::vector<Channel> channels;

    private:
        friend class AdapterPublishers;

        IdMask ids_allowed;
        IdMask ids_seen;
        bool accept_all_ids = true;
    };

    AdapterPublishers(){
        reserve_cache();
    };

    ~AdapterPublishers() override = default;

    PublisherShmConnection& shm() noexcept
    {
        return shm_;
    }

    const PublisherShmConnection& shm() const noexcept
    {
        return shm_;
    }

    void spin_once() override
    {
        fill_cache();
        dispatch();
    }

protected:
    PublisherShmConnection shm_;

    template<typename PublisherType>
    PublisherType& register_publisher(
        std::vector<Channel> channels,
        const std::vector<EcatId>& ids_allowed = {})
    {
        static_assert(std::is_base_of_v<IPublisher, PublisherType>, "PublisherType must derive from IPublisher");

        auto publisher = std::make_unique<PublisherType>();
        auto* publisher_ptr = publisher.get();

        Subscription subscription;
        subscription.publisher = publisher_ptr;
        subscription.channels = std::move(channels);
        subscription.accept_all_ids = ids_allowed.empty();

        for (const EcatId id : ids_allowed) {
            if (id >= MaxEcatIds) {
                LOG_ERROR(
                    "Configured ECAT ID {} exceeds maximum supported ID {}",
                    id,
                    MaxEcatIds - 1);
                continue;
            }

            if (subscription.ids_allowed.test(id)) {
                LOG_ERROR("Duplicate configured ECAT ID {}", id);
                continue;
            }

            subscription.ids_allowed.set(id);
        }

        subscriptions_.push_back(std::move(subscription));
        publishers_.push_back(std::move(publisher));

        return *publisher_ptr;
    }

    void dispatch(){
         for (auto& subscription : subscriptions_) {
            subscription.ids_seen.reset();
            subscription.publisher->begin_cycle();

            for (const Channel channel : subscription.channels) {
                const auto cache_it = cache_.find(channel);

                if (cache_it == cache_.end()) {
                    continue;
                }

                dispatch_cache(cache_it->second, subscription);
            }
            
            const bool valid = subscription.ids_seen == subscription.ids_allowed;
            subscription.publisher->end_cycle(valid);
        }
    }

    void reserve_cache()
    {
        cache_[Channel::Imu].reserve(2);
        cache_[Channel::Motor].reserve(32);
        cache_[Channel::Gripper].reserve(8);
        cache_[Channel::Pump].reserve(2);
        cache_[Channel::PowerBoard].reserve(2);
        cache_[Channel::ForceTorque].reserve(8);
    }

private:
    inline static constexpr std::array<Channel, static_cast<int>(Channel::Count)> all_channels_ {
        Channel::Imu,
        Channel::Motor,
        Channel::Gripper,
        Channel::Pump,
        Channel::PowerBoard,
        Channel::ForceTorque
    };

    std::vector<Subscription> subscriptions_;
    std::vector<std::unique_ptr<IPublisher>> publishers_;

    ShmProtoHelper proto_helper_;
    Cache cache_;

    void fill_cache()
    {
    for (const Channel channel : all_channels_) {
        auto& queue = shm_.resolve(channel);
        auto& output = cache_[channel];

        std::size_t valid_count = 0;
        ProtoSlot frame;

        while (queue.try_pop(frame)) {
            if (valid_count == output.size()) {
                output.emplace_back();
            }

            auto& cached = output[valid_count];

            cached.pdo.Clear();

            if (!pdo_utils::parse_frame(
                    frame.data,
                    static_cast<ssize_t>(frame.size),
                    cached.pdo)) {
                continue;
            }

            const int parsed_id =
                get_ecat_id(cached.pdo.header().str_id());

            if (parsed_id < 0) {
                LOG_ERROR(
                    "Format error for PDO frame with ID {}",
                    cached.pdo.header().str_id());
                continue;
            }

            const auto id = static_cast<EcatId>(parsed_id);

            if (id >= MaxEcatIds) {
                LOG_ERROR(
                    "Received ECAT ID {} exceeds maximum supported ID {}",
                    id,
                    MaxEcatIds - 1);
                continue;
            }

            cached.ecat_id = id;
            ++valid_count;
        }
        output.resize(valid_count);
    }
}

    static void dispatch_cache(const std::vector<CachedPdo>& pdos, Subscription& subscription)
    {
        for (const auto& frame : pdos) {
            const EcatId id = frame.ecat_id;

            if (!subscription.accept_all_ids &&
                !subscription.ids_allowed.test(id)) {
                continue;
            }

            if (subscription.ids_seen.test(id)) {
                continue;
            }

            subscription.ids_seen.set(id);
            subscription.publisher->consume(frame.pdo);
        }
    }
};

} // namespace middleware_adapter::message