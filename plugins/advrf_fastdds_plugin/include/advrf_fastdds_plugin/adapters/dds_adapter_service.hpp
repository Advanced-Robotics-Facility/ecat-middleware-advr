#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <advrf_interfaces/srv/ReplCmd.hpp>
#include <advrf_interfaces/srv/ReplCmdPubSubTypes.hpp>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

#include <advrf_middleware_core/adapters/adapter_service.hpp>

#include "advrf_fastdds_plugin/config/config_topics.hpp"
#include "advrf_fastdds_plugin/service/service_server.hpp"

using RequestDDS  = advrf_interfaces::srv::dds_::ReplCmd_Request_;
using ResponseDDS = advrf_interfaces::srv::dds_::ReplCmd_Response_;
using RequestDDSPubSubType  = advrf_interfaces::srv::dds_::ReplCmd_Request_PubSubType;
using ResponseDDSPubSubType = advrf_interfaces::srv::dds_::ReplCmd_Response_PubSubType;
using RequestProtobuf  = iit::advrf::Repl_cmd;
using ResponseProtobuf = iit::advrf::Cmd_reply;

class DDSAdapterService: public middleware_adapter::service::AdapterServiceServer
{
    
public:
    DDSAdapterService(const config::ConfigTopics& config_topics,
                     eprosima::fastdds::dds::DomainParticipant* participant);
    
    void spin_once() override { server_.spin_once(); }

private:
    ServiceServer<RequestDDS, RequestDDSPubSubType, ResponseDDS, ResponseDDSPubSubType> server_;
        
    ResponseDDS process_request_dds(const RequestDDS& request);
    ResponseProtobuf process_request_protobuf(const RequestProtobuf& request);
};