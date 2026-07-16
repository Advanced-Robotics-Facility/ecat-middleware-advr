#include "advrf_cyclonedds_plugin/service/service_server_cmd.hpp"


 ServiceServerCmd::ServiceServerCmd(const config::ConfigTopics& config_topics,
                     dds::domain::DomainParticipant& participant): 
            server_(participant,
                config_topics.srv.getCmdRequest(),
                config_topics.srv.getCmdReply())
{
    server_.set_callback(
        [this](const RequestDDS& request)
        {
            return process_cmd_(request);
        });

}

ResponseProtobuf ServiceServerCmd::process_cmd_(const RequestProtobuf& request)
{
    //TODO: fill shm stuff there
    ResponseProtobuf pb_reply; 
    return pb_reply;
}

ResponseDDS ServiceServerCmd::process_cmd_(const RequestDDS& request)
{
    RequestProtobuf pb_repl_cmd = convert::to_protobuf::convert_dds_to_protobuf(request);
    ResponseProtobuf pb_reply = process_cmd_(pb_repl_cmd);
    ResponseDDS dds_reply = convert::to_dds::convert_protobuf_to_dds(pb_reply);
    dds_reply.request_id(request.request_id());

    return dds_reply;
}