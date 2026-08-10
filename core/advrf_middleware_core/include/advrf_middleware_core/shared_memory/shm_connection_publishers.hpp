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

    Queue& resolve(ChannelRx channel)
    {
        auto it = k_map_channelrx_to_devicerx.find(channel);
        if(it == k_map_channelrx_to_devicerx.end()) {
            throw std::invalid_argument("Invalid channel");
        }
        return bridge().payload.queue_for(it->second);
    }

private:
};