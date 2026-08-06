#pragma once

#include <shm_types.hpp>

#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
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
        return proto_helper_.push(bridge().payload.queue_for(DeviceType::MOTOR), request);
    }

private:
    ShmProtoHelper proto_helper_;
};