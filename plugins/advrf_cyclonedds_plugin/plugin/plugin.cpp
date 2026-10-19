#include "advrf_cyclonedds_plugin/adapters/dds_adapter_service.hpp"
#include <dds/dds.hpp>
#include <iostream>
#include <memory>

#include <advrf_middleware_core/plugin/plugin_exec.hpp>

#include "advrf_cyclonedds_plugin/adapters/dds_adapter_publishers.hpp"
#include "advrf_cyclonedds_plugin/adapters/dds_adapter_subscribers.hpp"
#include "advrf_cyclonedds_plugin/config/plugin_config.hpp"

int main(int argc, char** argv)
{
    advrf::log::Log::init();
    using namespace advrf::middleware::config;
    using namespace advrf::cyclonedds_plugin;
    try
    {
        const auto options = parse_common_options(argc, argv);
        if (options.help)
        {
            std::cout << "Usage: cyclonedds_plugin [OPTIONS]\n" << common_options_help();
            return 0;
        }
        const auto path = options.config_path.empty()
                              ? advrf::middleware::config::middleware_config_path()
                              : options.config_path;
        const auto file = ConfigFile::load(path);
        auto config = parse_plugin_config(file);
        options.runtime.apply(config.common.runtime);
        advrf::cyclonedds_plugin::validate(config);
        const auto& robot = config.common.robot;
        const auto& runtime = config.common.runtime;
        ConfigTopics topics({robot.ns, robot.robot_name});

        advrf::middleware::ecat::EcatDiscover discover;
        if (!discover.start(SHM_NRT_RX_PDO))
        {
            LOG_ERROR("Failed to connect to EtherCAT PDO shared memory.");
            return 1;
        }

        const auto ecat_map = discover.discover(extract_pdo_ids(robot));
        dds::domain::DomainParticipant participant_pub(config.dds.domain_id);
        dds::domain::DomainParticipant participant_sub(config.dds.domain_id);
        dds::domain::DomainParticipant participant_service(config.dds.domain_id);

        auto publishers = std::make_shared<DDSAdapterPublishers>();
        if (!publishers->init(topics, robot, ecat_map, participant_pub, config.dds))
        {
            throw std::runtime_error("Failed to initialize DDS publishers");
        }

        auto subscribers = std::make_shared<DDSAdapterSubscribers>();
        if (!subscribers->init(topics, robot, ecat_map, participant_sub, config.dds))
        {
            throw std::runtime_error("Failed to initialize DDS subscribers");
        }

        auto service = std::make_shared<DDSAdapterService>(
            topics, robot, participant_service, config.dds);

        advrf::middleware::plugin::PluginExec exec;
        exec.register_adapter(
            {"dds_adapter_publishers", publishers, period_from_rate(runtime.rate_publishers)});
        exec.register_adapter(
            {"dds_adapter_subscribers", subscribers, period_from_rate(runtime.rate_subscribers)});
        exec.register_adapter({"dds_adapter_service", service,
            period_from_rate(runtime.rate_service)});
        LOG_INFO("Configuration: {}; DDS domain: {}; publishers: {} Hz; subscribers: {} Hz; "
                 "service: {} Hz",
            file.path.string(), config.dds.domain_id, runtime.rate_publishers,
            runtime.rate_subscribers, runtime.rate_service);
        exec.start();
        return 0;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("{}", e.what());
        return 1;
    }
}
