#pragma once

#include <dds/dds.hpp>

#include "advrf_cyclonedds_plugin/config/config_topics.hpp"
#include "advrf_cyclonedds_plugin/service/service_server.hpp"
#include "advrf_cyclonedds_plugin/converter.hpp"
#include "advrf_cyclonedds_plugin/service/shm_data.hpp"

#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <advrf_interfaces/srv/ReplCmd.hpp>
#include <sys/types.h>

using RequestDDS = advrf_interfaces::srv::dds_::ReplCmd_Request_;
using ResponseDDS = advrf_interfaces::srv::dds_::ReplCmd_Response_;
using RequestHeaderDDS = rcl_interfaces::msg::dds_::RequestHeader_;
using RequestProtobuf = iit::advrf::Repl_cmd;
using ResponseProtobuf = iit::advrf::Cmd_reply;
using RequestHeaderProtobuf = iit::advrf::Request_header;


class ServiceServerCmd
{
    
public:
    ServiceServerCmd(const config::ConfigTopics& config_topics,
                     dds::domain::DomainParticipant& participant);

private:
    ServiceServer<RequestDDS, ResponseDDS> server_;
    
    bool write_to_shm(const SHMBaseSrv& shm_content);
    bool read_from_shm(const SHMRequestInfo& shm_reference, ResponseProtobuf& shm_content);
    ResponseDDS process_cmd_dds(const RequestDDS& request);
};