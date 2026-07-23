#pragma once

#include <thread>

#include "advrf_middleware_core/utils/log.hpp"

#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

class ReplShmConnection
{
public:
    bool connect(const std::string& shm_name)
    {
        while (!stop_)
        {
            shm_ = std::make_unique<SharedMemoryClient>(
                shm_name.c_str(),
                sizeof(SharedReplBridge));

            if (shm_->is_valid())
                break;
            LOG_DEBUG("Wait connection shared memory, {}", shm_name);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        bridge_ = shm_->get<SharedReplBridge>();
        while (!stop_ && !bridge_->rt_ready.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        declare_ready();

        return true;
    }

    void declare_ready()
    {
        if (bridge_)
            bridge_->mw_ready.store(true);
    }

    void declare_not_ready()
    {
        if (bridge_)
            bridge_->mw_ready.store(false);
    }

    bool is_connected() const
    {
        return bridge_ != nullptr;
    }

    bool is_ok() const
    {
        return bridge_ && bridge_->rt_ready.load();
    }

    SharedReplBridge& bridge()
    {
        return *bridge_;
    }

    const SharedReplBridge& bridge() const
    {
        return *bridge_;
    }

    bool push_request(const iit::advrf::Repl_cmd& request)
    {
        return proto_helper_.push(bridge_->request, request);
    }

    bool pop_reply(ProtoSlot& frame)
    {
        return proto_helper_.pop_latest_frame(bridge_->reply, frame);
    }

    void close()
    {
        declare_not_ready();
        bridge_ = nullptr;
        stop_ = true;
    }

    auto& request_queue()
    {
        return bridge_->request;
    }

    auto& reply_queue()
    {
        return bridge_->reply;
    }

private:
    std::unique_ptr<SharedMemoryClient> shm_;
    SharedReplBridge* bridge_ = nullptr;
    bool stop_ = false;

    ShmProtoHelper proto_helper_;
};