#include <advrf_dds_common/converter/converter.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_fastdds_plugin/adapters/dds_adapter_service.hpp"

namespace advrf::fastdds_plugin {

DDSAdapterService::DDSAdapterService(const advrf::middleware::config::ConfigTopics& config_topics,
                                     const advrf::middleware::config::RobotConfig& robot_config,
                                     eprosima::fastdds::dds::DomainParticipant* participant,
                                     const advrf::dds_common::DdsConfig& dds_config)
    : AdapterServiceServer(dds_config.service),
      server_(participant, config_topics.service.request(), config_topics.service.reply())
{
    server_.set_callback([this](const RequestDDS& req) {
        return process_request_dds(req);
    });

    ros_connectables_.emplace_back(server_);
    init_ros_graph_bridge(robot_config, participant, dds_config.declare_to_ros);
}

ResponseDDS DDSAdapterService::process_request_dds(const RequestDDS& request)
{
    RequestProtobuf pb_req;
    advrf::dds_common::convert::protobuf::from_dds(request, pb_req);
    ResponseProtobuf pb_resp = this->process_request(pb_req);
    ResponseDDS resp;
    advrf::dds_common::convert::dds::from_protobuf(pb_resp, resp);
    resp.request_id() = request.request_id();
    return resp;
}


void DDSAdapterService::init_ros_graph_bridge(
    const advrf::middleware::config::RobotConfig& robot_config,
    eprosima::fastdds::dds::DomainParticipant* dp,
    bool declare_to_ros)   {
    if (!declare_to_ros) {
        return;
    }

    const auto node_namespace = FastRosGraphBridge::build_node_namespace(
        robot_config.ns,
        robot_config.robot_name
    );

    ros_graph_bridge_ = std::make_unique<FastRosGraphBridge>(
        dp,
        "service_node",
        node_namespace
    );

    for (auto& connectable : ros_connectables_) {
        connectable.get().connect_ros_graph_bridge(*ros_graph_bridge_);
    }
}

}