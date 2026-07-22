#pragma once

#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>
#include <thread>

#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

class ReplShmConnection
{
public:
    bool connect()
    {
        while (true)
        {
            shm_ = std::make_unique<SharedMemoryClient>(
                SHM_REPL_NAME,
                sizeof(SharedReplBridge));

            if (shm_->is_valid())
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        bridge_.reset(shm_->get<SharedReplBridge>());

        while (!bridge_->rt_ready.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        bridge_->mw_ready.store(true);

        return true;
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
    std::unique_ptr<SharedReplBridge> bridge_ = nullptr;

    ShmProtoHelper proto_helper_;
};