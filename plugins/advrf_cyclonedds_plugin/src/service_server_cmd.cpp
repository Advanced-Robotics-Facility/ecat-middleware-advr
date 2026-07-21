#include "advrf_cyclonedds_plugin/service/service_server_cmd.hpp"

ServiceServerCmd::ServiceServerCmd(const config::ConfigTopics& config_topics,
                                    dds::domain::DomainParticipant& participant)
    : server_(participant, config_topics.replCmd.request(), config_topics.replCmd.reply())
{
    
}

bool ServiceServerCmd::connect_dds(){
    server_.set_callback([this](const RequestDDS& req) {
        return process_request_dds(req);
    });
    return true;
}

bool ServiceServerCmd::connect_shm()
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

ResponseDDS ServiceServerCmd::process_request_dds(const RequestDDS& request)
{
    RequestProtobuf pb_req = convert::protobuf::from_dds(request);
    ResponseProtobuf pb_resp = process_request_protobuf(pb_req);
    ResponseDDS resp = convert::dds::from_protobuf(pb_resp);
    resp.request_id() = request.request_id();
    return resp;
}

ResponseProtobuf ServiceServerCmd::process_request_protobuf(const RequestProtobuf& request)
{    

    LOG_DEBUG("[ServiceServerCmd] Got DDS request, type={}", static_cast<int>(request.type()));
    ResponseProtobuf reply{};

    if (!repl_bridge_ || !repl_bridge_->rt_ready.load()) {
        LOG_ERROR("[ServiceServerCmd] repl_bridge_ null or rt_ready=false");
        reply.set_type(iit::advrf::Cmd_reply::NACK);
        reply.set_msg("ecat master not connected");
        return reply;
    }

    if (!proto_helper_.push(repl_bridge_->request, request)) {
        LOG_ERROR("[ServiceServerCmd] push to request queue FAILED");
        reply.set_type(iit::advrf::Cmd_reply::NACK);
        reply.set_msg("shm request queue full");
        return reply;
    }

    LOG_DEBUG("[ServiceServerCmd] pushed request to shm, waiting for reply...");

    ProtoSlot frame;
    const auto timeout = std::chrono::milliseconds(1000);
    const auto deadline = std::chrono::steady_clock::now() + timeout;

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