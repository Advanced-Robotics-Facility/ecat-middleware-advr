#pragma once

#include "advrf_middleware_core/shared_memory/shm_bridge_connection.hpp"

#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

class ReplShmConnection
    : public ShmBridgeConnection<ReplShmConnection, SharedReplBridge>
{
public:
    bool peer_ready() const
    {
        return bridge_->rt_ready.load();
    }

    void set_local_ready(bool ready)
    {
        bridge_->mw_ready.store(ready);
    }

    bool push_request(const iit::advrf::Repl_cmd& request)
    {
        return proto_helper_.push(bridge_->request, request);
    }

    bool pop_reply(ProtoSlot& frame)
    {
        return proto_helper_.pop_latest_frame(bridge_->reply, frame);
    }

private:
    ShmProtoHelper proto_helper_;
};