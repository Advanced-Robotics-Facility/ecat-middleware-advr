#include "advrf_cyclonedds_plugin/adapters/dds_adapter_motor_control.hpp"
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
                              ? ADVRF_CONFIG_SHARE / "middleware" / "config.yaml"
                              : options.config_path;
        const auto file = ConfigFile::load(path);
        auto config = parse_plugin_config(file);
        auto motor_config = advrf::middleware::adapters_mode::parse_motor_control_config(file);
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

        // Explicit permissions can narrow discovery, never add unknown devices.
        std::set<int32_t> discovered_motors;
        for (const auto& [id, metadata] : ecat_map)
        {
            if (metadata.type == iit::advrf::Ec_slave_pdo::RX_MOTOR ||
                metadata.type == iit::advrf::Ec_slave_pdo::RX_XT_MOTOR ||
                metadata.type == iit::advrf::Ec_slave_pdo::RX_CIA402)
            {
                discovered_motors.insert(static_cast<int32_t>(id));
            }
            if (metadata.type == iit::advrf::Ec_slave_pdo::RX_MOTOR ||
                metadata.type == iit::advrf::Ec_slave_pdo::RX_XT_MOTOR)
            {
                motor_config.advrf_motor_ids.insert(static_cast<int32_t>(id));
            }
        }
        if (motor_config.allowed_motor_ids.empty())
        {
            motor_config.allowed_motor_ids = discovered_motors;
        }
        else
        {
            for (const auto id : motor_config.allowed_motor_ids)
            {
                if (discovered_motors.count(id) == 0)
                {
                    throw std::runtime_error(
                        "motor_control: configured ID is not a discovered motor");
                }
            }
        }

        advrf::middleware::plugin::PluginExec exec;
        exec.register_adapter(
            {"dds_adapter_publishers", publishers, period_from_rate(runtime.rate_publishers)});
        exec.register_adapter(
            {"dds_adapter_subscribers", subscribers, period_from_rate(runtime.rate_subscribers)});
        if (motor_config.enabled)
        {
            if (motor_config.allowed_motor_ids.empty())
            {
                throw std::runtime_error("motor_control: no motors discovered; disable "
                                         "adapters_mode.motor_control for sensor-only robots");
            }
            auto motor_service =
                std::make_shared<DDSAdapterMotorControl>(topics, participant_service, motor_config);
            exec.register_adapter({"dds_adapter_motor_control", motor_service,
                period_from_rate(runtime.rate_service)});
            LOG_INFO("Motor control service: ZeroMQ {}, timeout {} ms", motor_config.command_uri,
                motor_config.timeout_ms);
        }
        // The legacy generic SHM service remains disabled pending the master-side fix.
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
