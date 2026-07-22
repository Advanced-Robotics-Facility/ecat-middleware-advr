#pragma once

#include "advrf_middleware_core/utils/log.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <ecat_master_future/shm_shared_types.hpp>

#include <ecat_master_future/shm_utils.hpp>

namespace middleware_adapter::service {

class AdapterServiceServer
{
public:

    AdapterServiceServer() = default;
    virtual ~AdapterServiceServer() = default;

    iit::advrf::Cmd_reply process_request(const iit::advrf::Repl_cmd& request)
    {
        iit::advrf::Cmd_reply reply;
        if (!bridge_ || !bridge_->rt_ready.load()) {
            LOG_ERROR("repl_bridge_ null or rt_ready=false");
            reply.set_type(iit::advrf::Cmd_reply::NACK);
            reply.set_msg("ecat master not connected");
            return reply;
        }

        if (!proto_helper_.push(bridge_->request, request)) {
            LOG_ERROR("push to request queue FAILED");
            reply.set_type(iit::advrf::Cmd_reply::NACK);
            reply.set_msg("shm request queue full");
            return reply;
        }

        LOG_DEBUG("pushed request to shm, waiting for reply...");

        ProtoSlot frame;
        const auto timeout = std::chrono::milliseconds(1000);
        const auto deadline = std::chrono::steady_clock::now() + timeout;

        while (std::chrono::steady_clock::now() < deadline) {
            if (proto_helper_.pop_latest_frame(bridge_->reply, frame)) {
                uint32_t payload_size = 0;
                if (ShmProtoHelper::frame_payload_size(frame, payload_size) &&
                    reply.ParseFromArray(frame.data + PROTO_FRAME_HEADER_BYTES,
                                        static_cast<int>(payload_size))) {
                    return reply;
                }
            }
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
        return reply;
    }

    bool shm_connect()
    {
        while (true) {
            shm_ = std::make_unique<SharedMemoryClient>(SHM_REPL_NAME, sizeof(SharedReplBridge));
            if (shm_->is_valid())
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        bridge_.reset(shm_->get<SharedReplBridge>());
        while (!bridge_->rt_ready.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        bridge_->mw_ready.store(true);
        return true;
    }


protected:
    std::unique_ptr<SharedMemoryClient> shm_ = nullptr;
    std::unique_ptr<SharedReplBridge> bridge_ = nullptr;

private:
    ShmProtoHelper proto_helper_;

};

}