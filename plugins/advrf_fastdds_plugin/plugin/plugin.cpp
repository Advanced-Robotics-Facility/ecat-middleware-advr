#include <iostream>
#include <memory>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <advrf_middleware_core/plugin/plugin_exec.hpp>

#include "advrf_fastdds_plugin/adapters/dds_adapter_publishers.hpp"
#include "advrf_fastdds_plugin/adapters/dds_adapter_service.hpp"
#include "advrf_fastdds_plugin/adapters/dds_adapter_subscribers.hpp"
#include "advrf_fastdds_plugin/config/plugin_config.hpp"

int main(int argc, char** argv) {
    advrf::log::Log::init();
    using namespace advrf::middleware::config;
    using namespace advrf::fastdds_plugin;
    try {
        const auto options = parse_common_options(argc, argv);
        if (options.help) {
            std::cout << "Usage: fastdds_plugin [OPTIONS]\n" << common_options_help();
            return 0;
        }
        const auto path = options.config_path.empty()
            ? ADVRF_CONFIG_SHARE / "middleware" / "config.yaml" : options.config_path;
        const auto file = ConfigFile::load(path);
        auto config = parse_plugin_config(file);
        options.runtime.apply(config.common.runtime);
        advrf::fastdds_plugin::validate(config);
        const auto& robot = config.common.robot;
        const auto& runtime = config.common.runtime;
        ConfigTopics topics({robot.ns, robot.robot_name});

        advrf::middleware::ecat::EcatDiscover discover;
        if (!discover.start(SHM_NRT_RX_PDO)) {
            LOG_ERROR("Failed to connect to EtherCAT PDO shared memory.");
            return 1;
        }
        const auto ecat_map = discover.discover(extract_pdo_ids(robot));
        // Participant owners precede adapters so DDS entities are destroyed first.
        auto make_participant = [&]() {
            auto* participant = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()
                ->create_participant(config.dds.domain_id, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
            if (!participant) throw std::runtime_error("Failed to create DDS participant");
            auto cleanup = [](eprosima::fastdds::dds::DomainParticipant* dp) {
                dp->delete_contained_entities();
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(dp);
            };
            return std::unique_ptr<eprosima::fastdds::dds::DomainParticipant, decltype(cleanup)>(participant, cleanup);
        };
        auto participant_pub = make_participant();
        auto participant_sub = make_participant();
        auto participant_service = make_participant();
        auto publishers = std::make_shared<DDSAdapterPublishers>();
        if (!publishers->init(topics, robot, ecat_map, participant_pub.get(), config.dds))
            throw std::runtime_error("Failed to initialize DDS publishers");
        auto subscribers = std::make_shared<DDSAdapterSubscribers>();
        if (!subscribers->init(topics, robot, ecat_map, participant_sub.get(), config.dds))
            throw std::runtime_error("Failed to initialize DDS subscribers");
        auto service = std::make_shared<DDSAdapterService>(topics, robot, participant_service.get(), config.dds);

        advrf::middleware::plugin::PluginExec exec;
        exec.register_adapter({"dds_adapter_publishers", publishers, period_from_rate(runtime.rate_publishers)});
        exec.register_adapter({"dds_adapter_subscribers", subscribers, period_from_rate(runtime.rate_subscribers)});
        // Service execution remains disabled pending the master-side fix.
        // exec.register_adapter({"dds_adapter_service", service, period_from_rate(runtime.rate_service)});
        LOG_INFO("Configuration: {}; DDS domain: {}; publishers: {} Hz; subscribers: {} Hz; service: {} Hz",
                 file.path.string(), config.dds.domain_id, runtime.rate_publishers,
                 runtime.rate_subscribers, runtime.rate_service);
        exec.start();
        return 0;
    } catch (const std::exception& e) {
        LOG_ERROR("{}", e.what());
        return 1;
    }
}
