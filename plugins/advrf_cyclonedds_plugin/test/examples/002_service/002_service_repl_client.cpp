#include <dds/dds.hpp>
#include <filesystem>

#include <advrf_interfaces/srv/ReplCmd.hpp>
#include <advrf_middleware_core/utils/log.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>

#include "advrf_cyclonedds_plugin/service/service_client.hpp"
#include "advrf_middleware_core/config/config_topics.hpp"

using RequestDDS = advrf_interfaces::srv::dds_::ReplCmd_Request_;
using ResponseDDS = advrf_interfaces::srv::dds_::ReplCmd_Response_;

int main()
{
    advrf::log::Log::init();
    auto config_robot = config::load_robot_config(
        ADVRF_CONFIG_SHARE / "robot_id_map" / "robot_id_map.yaml",
        ADVRF_CONFIG_SHARE / "robot_ecat" / "ecat_config.yaml");
    if (!config_robot)
        return 1;

    auto dds_participant = dds::domain::DomainParticipant{config_robot->domain_id};
    auto config = config::ConfigTopics{{config_robot->ns, config_robot->robot_name}};
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