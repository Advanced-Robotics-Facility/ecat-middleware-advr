#pragma once

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/log.hpp"
#include "advrf_middleware_core/shared_memory/shm_connection_repl.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>

namespace middleware_adapter::service   {   

class AdapterServiceServer : public AdapterBase
{
public:

    AdapterServiceServer() = default;
    virtual ~AdapterServiceServer() = default;

    iit::advrf::Cmd_reply process_request(const iit::advrf::Repl_cmd& request)
    {
        iit::advrf::Cmd_reply reply;
        if (!shm_.is_ok())
        {
            LOG_ERROR("RT not connected");
            reply.set_type(iit::advrf::Cmd_reply::NACK);
            reply.set_msg("ecat master not connected");
            return reply;
        }

        if (!shm_.push_request(request))
        {
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
            if (shm_.pop_reply(frame)) {
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


    ReplShmConnection& shm()
    {
        return shm_;
    }

    const ReplShmConnection& shm() const
    {
        return shm_;
    }


protected:
    ReplShmConnection shm_;

private:
    ShmProtoHelper proto_helper_;

};

}