#include "advrf_cyclonedds_plugin/service/service_server_cmd.hpp"
#include <ecat_master_future/shm_utils.hpp>
#include <ecat_master_future/shm_shared_types.hpp>
#include <chrono>
#include <thread>
#include <iostream>

bool ServiceServerCmd::connect_shm_()
{
    while (true) {
        repl_shm_ = std::make_unique<SharedMemoryClient>(SHM_REPL_NAME, sizeof(SharedReplBridge));
        if (repl_shm_->is_valid())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    repl_bridge_ = repl_shm_->get<SharedReplBridge>();

    while (!repl_bridge_->rt_ready.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    repl_bridge_->mw_ready.store(true);
    return true;
}

ServiceServerCmd::ServiceServerCmd(const config::ConfigTopics& config_topics,
                                    dds::domain::DomainParticipant& participant)
    : server_(participant, config_topics.replCmd.request(), config_topics.replCmd.reply())
{
    connect_shm_();

    server_.set_callback([this](const RequestDDS& req) {
        return process_cmd_(req);
    });
}

bool ServiceServerCmd::read_from_shm(const SHMRequestInfo& shm_request_info, ResponseProtobuf& shm_output)
{
    RequestProtobuf pb_req = convert::to_protobuf::convert_dds_to_protobuf(request);
    ResponseProtobuf pb_resp = process_cmd_(pb_req);
    ResponseDDS resp = convert::to_dds::convert_protobuf_to_dds(pb_resp);
    resp.request_id() = request.request_id();
    return resp;
}

ResponseProtobuf ServiceServerCmd::process_cmd_(const RequestProtobuf& request)
{    
    std::cerr << "[ServiceServerCmd] Got DDS request, type=" << request.type() << std::endl;
    ResponseProtobuf reply{};

    if (!repl_bridge_ || !repl_bridge_->rt_ready.load()) {
        std::cerr << "[ServiceServerCmd] repl_bridge_ null or rt_ready=false" << std::endl;
        reply.set_type(iit::advrf::Cmd_reply::NACK);
        reply.set_msg("ecat master not connected");
        return reply;
    }

    if (!proto_helper_.push(repl_bridge_->request, request)) {
        std::cerr << "[ServiceServerCmd] push to request queue FAILED" << std::endl;
        reply.set_type(iit::advrf::Cmd_reply::NACK);
        reply.set_msg("shm request queue full");
        return reply;
    }

    std::cerr << "[ServiceServerCmd] pushed request to shm, waiting for reply..." << std::endl;


    ProtoSlot frame;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);

    while (std::chrono::steady_clock::now() < deadline) {
        if (proto_helper_.pop_latest_frame(repl_bridge_->reply, frame)) {
            uint32_t payload_size = 0;
            if (ShmProtoHelper::frame_payload_size(frame, payload_size) &&
                reply.ParseFromArray(frame.data + PROTO_FRAME_HEADER_BYTES,
                                      static_cast<int>(payload_size))) {
                return reply;
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }

    reply.set_type(iit::advrf::Cmd_reply::NACK);
    reply.set_msg("timeout waiting for ecat master reply");
    return reply;
}