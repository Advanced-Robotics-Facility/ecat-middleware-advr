#pragma once

#include "advrf_middleware_core/utils/channel.hpp"
#include <shm_types.hpp>

#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/shared_memory/shm_bridge_connection.hpp>

class SubscriberShmConnection
    : public ShmMiddlewareBridgeConnection<SubscriberShmConnection, SharedProtoSubBridge>
{
public:
    bool remote_ready() const
    {
        return bridge().status.shm_ready.load(std::memory_order_acquire);
    }

    // TODO: forward to the different queues
    bool push_request(const iit::advrf::Repl_cmd& request)
    {
        return proto_helper_.push(bridge().payload.queue_for(DeviceTypeTx::MOTOR), request);
    }

    bool push_pdo(const iit::advrf::Ec_slave_pdo& pdo, ChannelTx channel)
    {
        auto it = k_map_channeltx_to_devicetx.find(channel);
        if(it == k_map_channeltx_to_devicetx.end()) {
            throw std::invalid_argument("Invalid channel");
        }
        return proto_helper_.push(bridge().payload.queue_for(it->second), pdo);
    }

private:
    ShmProtoHelper proto_helper_;
};