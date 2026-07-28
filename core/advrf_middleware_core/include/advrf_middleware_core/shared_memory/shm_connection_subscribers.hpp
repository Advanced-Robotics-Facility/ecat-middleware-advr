#pragma once

#include "advrf_middleware_core/shared_memory/shm_bridge_connection.hpp"

#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

class SubscriberShmConnection
    : public ShmBridgeConnection<SubscriberShmConnection, SharedSubBridge>
{
public:
    bool peer_ready() const
    {
        return true;
    }

    void set_local_ready(bool ready)
    {
        bridge_->mw_ready.store(ready);
    }

    bool push_request(const iit::advrf::Repl_cmd& request)
    {
        return proto_helper_.push(bridge_->request, request);
    }

private:
    ShmProtoHelper proto_helper_;
};