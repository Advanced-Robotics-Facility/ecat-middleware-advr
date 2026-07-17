#include "advrf_cyclonedds_plugin/service/service_server_cmd.hpp"
#include <cfloat>


 ServiceServerCmd::ServiceServerCmd(const config::ConfigTopics& config_topics,
                     dds::domain::DomainParticipant& participant): 
            server_(participant,
                config_topics.srv.getCmdRequest(),
                config_topics.srv.getCmdReply())
{
    server_.set_callback(
        [this](const RequestDDS& request)
        {
            return process_cmd_dds(request);
        });

}


bool ServiceServerCmd::write_to_shm(const SHMBaseSrv& shm_content)
{
    return true;
}

bool ServiceServerCmd::read_from_shm(const SHMRequestInfo& shm_request_info, ResponseProtobuf& shm_output)
{
    const SHMBaseSrv shm_recv; // TODO: read from shm
    const bool is_same_req = true; //TODO: shm_recv.request_info.header == shm_reference.request_info.header;
    const bool is_ready = true; // TODO: shm_recv.request_info.status == READY;
    if(is_same_req && is_ready) {
        // TODO: convert from shm to protobuf
        shm_output = convert::protobuf::from_shm(shm_recv);
        return true;
    }
    return false;
}

ResponseDDS ServiceServerCmd::process_cmd_dds(const RequestDDS& request)
{
    // convert to protobuf
    RequestProtobuf pb_repl_cmd = convert::protobuf::from_dds(request);
    RequestHeaderProtobuf pb_request_header = convert::protobuf::from_dds(request.request_id());
    
    ResponseDDS dds_reply;
    ResponseProtobuf pb_reply;

    // convert to shm
    SHMBaseSrv shm_repl_cmd;
    convert::shm::from_protobuf(pb_repl_cmd, shm_repl_cmd); // TODO

    // write to shm
    bool shm_write_success = write_to_shm(shm_repl_cmd);
    if (!shm_write_success)
    {
        dds_reply.request_id(request.request_id());
        dds_reply.success(false);
        dds_reply.msg("Failed to write to shared memory");
        return dds_reply;
    }

    // wait for response from shm
    bool is_timeout = false; // TODO
    while (!is_timeout && !read_from_shm(shm_repl_cmd.request_info, pb_reply))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // handle is timeout case
    if(is_timeout)
    {
        dds_reply.request_id(request.request_id());
        dds_reply.success(false);
        dds_reply.msg("Timeout while waiting for response from shared memory");
        // TODO clear shm;
        return dds_reply;
    }

    // convert from shm to protobuf

    dds_reply = convert::dds::from_protobuf(pb_reply);
    dds_reply.request_id(request.request_id());
    dds_reply.success(true);

    return dds_reply;
}