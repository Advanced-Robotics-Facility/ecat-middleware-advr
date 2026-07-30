#pragma once

// ecat_master_future
#include <ecat_master_future/shm/bridge_struct.hpp>
#include <ecat_master_future/shm/proto_helper.hpp>

// advrf_middleware_core
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <advrf_middleware_core/shared_memory/shm_bridge_connection.hpp>


class ReplShmConnection
    : public ShmMiddlewareBridgeConnection<ReplShmConnection, SharedReplBridge>
{
public:
    bool remote_ready() const
    {
        return bridge().status.rt_ready.load();
    }

    void set_ready(bool ready)
    {
        bridge().status.mw_ready.store(ready);
    }

    bool push_request(const iit::advrf::Repl_cmd& request)
    {
        return proto_helper_.push(bridge().payload.request, request);
    }

    bool pop_reply(ProtoSlot& frame)
    {
        return proto_helper_.pop_latest_frame(bridge().payload.reply, frame);
    }

private:
    ShmProtoHelper proto_helper_;
};