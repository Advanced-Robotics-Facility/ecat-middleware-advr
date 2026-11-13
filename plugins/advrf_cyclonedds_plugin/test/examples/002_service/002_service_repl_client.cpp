#include <filesystem>
#include <dds/dds.hpp>

#include <advrf_ros2_interfaces/srv/ReplCmd.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_cyclonedds_plugin/config/plugin_config.hpp"
#include "advrf_cyclonedds_plugin/service/service_client.hpp"

using RequestDDS = advrf_ros2_interfaces::srv::dds_::ReplCmd_Request_;
using ResponseDDS = advrf_ros2_interfaces::srv::dds_::ReplCmd_Response_;

int main()
{
    advrf::log::Log::init();
    auto plugin_config = advrf::cyclonedds_plugin::parse_plugin_config(
        advrf::middleware::config::ConfigFile::load(advrf::middleware::config::middleware_config_path()));
    advrf::cyclonedds_plugin::validate(plugin_config);
    const auto* config_robot = &plugin_config.common.robot;
    auto dds_participant = dds::domain::DomainParticipant{plugin_config.dds.domain_id};
    auto config = advrf::middleware::config::ConfigTopics{{config_robot->ns, config_robot->robot_name}};
    advrf::cyclonedds_plugin::ServiceClient<RequestDDS, ResponseDDS> client(
        dds_participant, config.service.request(), 
        config.service.reply());

    RequestDDS request{};
    request.request().type() = static_cast<uint8_t>(4); // ECAT_MASTER_CMD, matching CmdType enum value
    request.request().ecat_master_cmd().type() = static_cast<uint8_t>(3); 
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    LOG_INFO("Sending request...");
    auto response = client.call(request, std::chrono::milliseconds(2000));

    if (response) {
        LOG_INFO("Got reply. type={} msg={}", static_cast<int>(response->type()), response->msg());
    } else {
        LOG_WARN("No reply (timeout).");
    }

    return 0;
}