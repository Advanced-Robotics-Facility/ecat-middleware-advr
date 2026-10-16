#include <advrf_cyclonedds_plugin/adapters/dds_adapter_motor_control.hpp>

namespace advrf::cyclonedds_plugin
{

DDSAdapterMotorControl::DDSAdapterMotorControl(
    const advrf::middleware::config::ConfigTopics& topics,
    dds::domain::DomainParticipant& participant,
    advrf::middleware::adapters_mode::MotorControlConfig config)
    : MotorControlAdapter(std::move(config)),
      ros_server_(participant, topics.service.motorControlRosRequest(),
          topics.service.motorControlRosReply())
{
    ros_server_.set_callback([this](const RosMotorRequestDDS& request)
    {
        MotorRequestDDS command;
        command.client_id("ros-" + std::to_string(request.request_id().guid()));
        command.request_id(static_cast<uint64_t>(request.request_id().seq()));
        command.motor_id(request.motor_id());
        command.command(request.command());
        command.control_mode(request.control_mode());
        command.gains(request.gains());
        const auto result = process(command);
        RosMotorReplyDDS response;
        response.request_id(request.request_id());
        response.success(result.success());
        response.outcome_unknown(result.outcome_unknown());
        response.message(result.message());
        return response;
    });
    // Announce the DDS service endpoints in the ROS graph; no separate ROS node
    // or client-side transport adapter is required.
    ros_graph_ = std::make_unique<CycloneDDSRosGraphBridge>(
        participant, "motor_control_service", "/middleware");
    ros_graph_->add_reader(ros_server_.dds_reader());
    ros_graph_->add_writer(ros_server_.dds_writer());
}

void DDSAdapterMotorControl::spin_once()
{
    ros_server_.spin_once();
}

MotorReplyDDS DDSAdapterMotorControl::process(const MotorRequestDDS& request)
{
    const Key key{request.client_id(), request.request_id()};
    const auto cached = replies_.find(key);
    if (cached != replies_.end())
    {
        return cached->second;
    }

    MotorReplyDDS response;
    response.client_id(request.client_id());
    response.request_id(request.request_id());
    response.motor_id(request.motor_id());

    if (request.client_id().empty() || request.client_id().size() > 128 ||
        request.request_id() == 0)
    {
        response.success(false);
        response.outcome_unknown(false);
        response.message("Provide a client ID (1..128 characters) and a nonzero request ID");
        return response;
    }

    advrf::middleware::adapters_mode::MotorControlRequest command;
    command.motor_id = request.motor_id();
    command.command =
        static_cast<advrf::middleware::adapters_mode::MotorCommand>(request.command());
    command.control_mode = request.control_mode();
    command.gains = request.gains();
    const auto result = execute(command);
    response.success(result.success);
    response.outcome_unknown(result.outcome_unknown);
    response.message(result.message);

    // Suppress duplicate execution for recent retried DDS requests, including
    // requests whose ZeroMQ result timed out. Never retry a motor command here.
    replies_.emplace(key, response);
    order_.push_back(key);
    if (order_.size() > 256)
    {
        replies_.erase(order_.front());
        order_.pop_front();
    }

    return response;
}

} // namespace advrf::cyclonedds_plugin
