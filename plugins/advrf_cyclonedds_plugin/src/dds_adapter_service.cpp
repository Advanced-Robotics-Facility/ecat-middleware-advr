#include "advrf_cyclonedds_plugin/adapters/dds_adapter_service.hpp"
#include "advrf_dds_common/converter/converter.hpp"

#include <dds/dds.hpp>
#include <sys/types.h>

#include <advrf_middleware_core/utils/log.hpp>

DDSAdapterService::DDSAdapterService(const config::ConfigTopics& config_topics,
                                     const RobotConfig& robot_config,
                                    dds::domain::DomainParticipant& participant)
    : server_(participant, config_topics.service.request(), config_topics.service.reply())
{
    server_.set_callback([this](const RequestDDS& req) {
        std::cerr << "Received request with ID: " << req.request_id() << std::endl;
        return process_request_dds(req);
    });

    ros_connectables_.emplace_back(server_);
    init_ros_graph_bridge(robot_config, participant);
}

ResponseDDS DDSAdapterService::process_request_dds(const RequestDDS& request)
{
    RequestProtobuf pb_req;
    convert::protobuf::from_dds(request, pb_req);
    ResponseProtobuf pb_resp = this->process_request(pb_req);
    ResponseDDS resp;
    convert::dds::from_protobuf(pb_resp, resp);
    resp.request_id() = request.request_id();
    return resp;
}


void DDSAdapterService::init_ros_graph_bridge(
    const RobotConfig& robot_config,
    dds::domain::DomainParticipant& dp)
{
    if (!robot_config.declare_to_ros) {
        return;
    }

    const auto node_namespace = CycloneDDSRosGraphBridge::build_node_namespace(
        robot_config.ns,
        robot_config.robot_name
    );

    ros_graph_bridge_ = std::make_unique<CycloneDDSRosGraphBridge>(
        dp,
        "service_node",
        node_namespace
    );

    for (auto& connectable : ros_connectables_) {
        connectable.get().connect_ros_graph_bridge(*ros_graph_bridge_);
    }
}