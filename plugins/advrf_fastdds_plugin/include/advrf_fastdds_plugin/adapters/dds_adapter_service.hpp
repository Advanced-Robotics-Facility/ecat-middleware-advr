#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <advrf_interfaces/srv/ReplCmd.hpp>
#include <advrf_interfaces/srv/ReplCmdPubSubTypes.hpp>
#include <advrf_middleware_core/adapters/adapter_service.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>

#include "advrf_fastdds_plugin/service/service_server.hpp"
#include "advrf_fastdds_plugin/ros_metadata/ros_graph_bridge.hpp"

namespace advrf::fastdds_plugin {

using RequestDDS  = advrf_interfaces::srv::dds_::ReplCmd_Request_;
using ResponseDDS = advrf_interfaces::srv::dds_::ReplCmd_Response_;
using RequestDDSPubSubType  = advrf_interfaces::srv::dds_::ReplCmd_Request_PubSubType;
using ResponseDDSPubSubType = advrf_interfaces::srv::dds_::ReplCmd_Response_PubSubType;
using RequestProtobuf  = iit::advrf::Repl_cmd;
using ResponseProtobuf = iit::advrf::Cmd_reply;

template <typename RequestDDS, typename RequestDDSPubSubType, typename ResponseDDS, typename ResponseDDSPubSubType>
class DDSAdapterBridgeService
    : public IConnectRosGraphBridge,
      public ServiceServer<RequestDDS, RequestDDSPubSubType, ResponseDDS, ResponseDDSPubSubType>
{
public:

    DDSAdapterBridgeService(
        eprosima::fastdds::dds::DomainParticipant* participant,
        const std::string& request_topic_name,
        const std::string& reply_topic_name)
        : ServiceServer<RequestDDS, RequestDDSPubSubType, ResponseDDS, ResponseDDSPubSubType>(
            participant,
            request_topic_name,
            reply_topic_name)
    {}

    ~DDSAdapterBridgeService() override = default;

    void connect_ros_graph_bridge(FastRosGraphBridge& bridge) override
    {
        bridge.add_reader(this->dds_reader());
        bridge.add_writer(this->dds_writer());
    }
};


class DDSAdapterService: public middleware_adapter::service::AdapterServiceServer
{
    
public:
    DDSAdapterService(const config::ConfigTopics& config_topics,
                      const config::RobotConfig& robot_config,
                      eprosima::fastdds::dds::DomainParticipant* participant);
    
    void spin_once() override { server_.spin_once(); }

private:
    DDSAdapterBridgeService<RequestDDS, RequestDDSPubSubType, ResponseDDS, ResponseDDSPubSubType> server_;
        
    ResponseDDS process_request_dds(const RequestDDS& request);
    ResponseProtobuf process_request_protobuf(const RequestProtobuf& request);

    void init_ros_graph_bridge(
        const config::RobotConfig& robot_config,
        eprosima::fastdds::dds::DomainParticipant* dp
    );

    std::unique_ptr<FastRosGraphBridge> ros_graph_bridge_;
    std::vector<std::reference_wrapper<IConnectRosGraphBridge>> ros_connectables_;
};

}