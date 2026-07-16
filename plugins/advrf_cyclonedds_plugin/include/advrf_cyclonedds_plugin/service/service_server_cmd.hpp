#pragma once

#include <dds/dds.hpp>

#include "advrf_cyclonedds_plugin/config/config_topics.hpp"
#include "advrf_cyclonedds_plugin/service/service_server.hpp"
#include "advrf_cyclonedds_plugin/converter.hpp"

#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <advrf_interfaces/srv/ReplCmd.hpp>

using RequestDDS = advrf_interfaces::srv::dds_::ReplCmd_Request_;
using ResponseDDS = advrf_interfaces::srv::dds_::ReplCmd_Response_;
using RequestProtobuf = iit::advrf::Repl_cmd;
using ResponseProtobuf = iit::advrf::Cmd_reply;

class ServiceServerCmd
{
    
public:
    ServiceServerCmd(const config::ConfigTopics& config_topics,
                     dds::domain::DomainParticipant& participant);

private:
    ServiceServer<RequestDDS, ResponseDDS> server_;

    ResponseProtobuf process_cmd_(const RequestProtobuf& request);
    ResponseDDS process_cmd_(const RequestDDS& request);
};