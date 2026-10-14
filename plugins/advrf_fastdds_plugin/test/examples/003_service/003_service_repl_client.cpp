#include <chrono>
#include <filesystem>
#include <thread>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <advrf_interfaces/srv/ReplCmd.hpp>
#include <advrf_interfaces/srv/ReplCmdPubSubTypes.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_fastdds_plugin/config/plugin_config.hpp"
#include "advrf_fastdds_plugin/service/service_client.hpp"

using RequestDDS = advrf_interfaces::srv::dds_::ReplCmd_Request_;
using RequestPubSubType = advrf_interfaces::srv::dds_::ReplCmd_Request_PubSubType;
using ResponseDDS = advrf_interfaces::srv::dds_::ReplCmd_Response_;
using ResponsePubSubType = advrf_interfaces::srv::dds_::ReplCmd_Response_PubSubType;

int main()
{
    advrf::log::Log::init();

    auto* participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        42,
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT
    );
    if (participant == nullptr) {
        LOG_ERROR("Failed to create DDS participant");
        return 1;
    }

    auto plugin_config = advrf::fastdds_plugin::parse_plugin_config(
        advrf::middleware::config::ConfigFile::load(ADVRF_CONFIG_SHARE / "middleware" / "config.yaml"));
    advrf::fastdds_plugin::validate(plugin_config);
    const auto* config_robot = &plugin_config.common.robot;
    advrf::middleware::config::ConfigTopics config_topics({config_robot->ns, config_robot->robot_name});

    advrf::fastdds_plugin::ServiceClient<RequestDDS, RequestPubSubType, ResponseDDS, ResponsePubSubType> client(
        participant,
        config_topics.service.request(),
        config_topics.service.reply()
    );

    RequestDDS request{};
    request.request().type() = static_cast<uint8_t>(4);
    request.request().ecat_master_cmd().type() = static_cast<uint8_t>(3);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    LOG_INFO("Sending request...");

    auto response = client.call(request, std::chrono::milliseconds(2000));
    if (response) {
        LOG_INFO("Got reply. type={} msg={}", static_cast<int>(response->type()), response->msg());
    } else {
        LOG_WARN("No reply (timeout).");
    }

    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);

    return 0;
}