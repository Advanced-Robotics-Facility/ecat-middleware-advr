#pragma once

#include <dds/dds.hpp>
#include <sys/types.h>
#include <chrono>

#include "advrf_cyclonedds_plugin/config/config_topics.hpp"
#include "advrf_cyclonedds_plugin/service/service_server.hpp"
#include "advrf_cyclonedds_plugin/converter.hpp"

#include <ecat_master_future/shm_utils.hpp>
#include <ecat_master_future/shm_shared_types.hpp>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <advrf_interfaces/srv/ReplCmd.hpp>
#include <advrf_middleware_core/utils/log.hpp>
#include <advrf_middleware_core/adapters/adapter_service.hpp>

using RequestDDS = advrf_interfaces::srv::dds_::ReplCmd_Request_;
using ResponseDDS = advrf_interfaces::srv::dds_::ReplCmd_Response_;
using RequestProtobuf = iit::advrf::Repl_cmd;
using ResponseProtobuf = iit::advrf::Cmd_reply;

class DDSAdapterService: public middleware_adapter::service::AdapterServiceServer
{
    
public:
    DDSAdapterService(const config::ConfigTopics& config_topics,
                     dds::domain::DomainParticipant& participant);
    
    void spin_once() { server_.spin_once(); }
    void spin(std::chrono::milliseconds period = std::chrono::milliseconds(10)) {
        server_.spin(period);
    }

private:
    ServiceServer<RequestDDS, ResponseDDS> server_;
        
    ResponseDDS process_request_dds(const RequestDDS& request);
    ResponseProtobuf process_request_protobuf(const RequestProtobuf& request);
};