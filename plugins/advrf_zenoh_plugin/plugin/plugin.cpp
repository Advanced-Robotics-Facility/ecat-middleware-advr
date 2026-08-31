#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include <zenoh.hxx>

#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/plugin/plugin_exec.hpp>
#include <advrf_middleware_core/utils/log.hpp>

#include "advrf_zenoh_plugin/adapters/zenoh_adapter_publishers.hpp"
#include "advrf_zenoh_plugin/adapters/zenoh_adapter_subscribers.hpp"
#include "advrf_zenoh_plugin/config/wire_format.hpp"
//#include "advrf_zenoh_plugin/adapters/zenoh_adapter_services.hpp"
namespace
{

using advrf::zenoh_plugin::WireFormat;

struct Options
{
    std::uint32_t rate_publishers = 1000;
    std::uint32_t rate_service = 10;
    std::uint32_t rate_subscribers = 100;
    WireFormat wire_format = WireFormat::Protobuf;
    std::string zenoh_config;
};

WireFormat parse_wire_format(const std::string& value)
{
    if (value == "protobuf")
        return WireFormat::Protobuf;

    if (value == "ros2cdr")
    {
#ifndef ZENOH_ROS2_SUPPORT
        throw std::runtime_error("Wire format 'ros2cdr' is unavailable: rebuild with ""-DZENOH_ROS2_SUPPORT=ON");
#else
        return WireFormat::Ros2Cdr;
#endif
    }

    throw std::runtime_error("Invalid wire format '" + value + "'. Expected 'protobuf' or 'ros2cdr'.");
}

Options parse_args(int argc, char** argv)
{
    Options options;

    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];

        auto read_value = [&]() -> std::string {
            if (index + 1 >= argc) {
                throw std::runtime_error("Missing value after " + argument);
            }
            return argv[++index];
        };

        auto read_rate = [&](std::uint32_t& rate) {
            rate = static_cast<std::uint32_t>(std::stoul(read_value()));
        };

        if (argument == "--rate-publishers")
            read_rate(options.rate_publishers);
        else if (argument == "--rate-service")
            read_rate(options.rate_service);
        else if (argument == "--rate-subscribers")
            read_rate(options.rate_subscribers);
        else if (argument == "--wire-format")
            options.wire_format = parse_wire_format(read_value());
        else if (argument == "--zenoh-config")
            options.zenoh_config = read_value();
        else if (argument == "--help" || argument == "-h")
        {
            std::cout
                << "Usage:\n"
                << "  zenoh_plugin [OPTIONS]\n\n"
                << "Options:\n"
                << "  --rate-publishers <Hz>   Publisher frequency (default: 1000)\n"
                << "  --rate-service <Hz>      Service health frequency (default: 10)\n"
                << "  --rate-subscribers <Hz>  Subscriber health frequency (default: 100)\n"
                << "  --wire-format <FORMAT>   protobuf or ros2cdr (default: protobuf)\n"
                << "  --zenoh-config <PATH>    Zenoh JSON5 configuration file\n";
            std::exit(0);
        }
        else
        {
            throw std::runtime_error("Unknown argument: " + argument);
        }
    }

    return options;
}

zenoh::Config make_zenoh_config(const Options& options)
{
    if (!options.zenoh_config.empty())
        return zenoh::Config::from_file(options.zenoh_config);

    if (std::getenv("ZENOH_CONFIG") != nullptr)
        return zenoh::Config::from_env();

    return zenoh::Config::create_default();
}

std::chrono::microseconds period_from_rate(std::uint32_t rate)
{
    if (rate == 0)
        throw std::runtime_error("Rate must be greater than zero.");

    return std::chrono::microseconds(1'000'000 / rate);
}

}

int main(int argc, char** argv)
{
    advrf::log::Log::init();

    try
    {
        const auto options = parse_args(argc, argv);
        auto robot = config::load_robot_config(
            ADVRF_CONFIG_SHARE / "robot_id_map" / "robot_id_map.yaml",
            ADVRF_CONFIG_SHARE / "robot_ecat" / "ecat_config.yaml");
        if (!robot)
            return 1;

        EcatDiscover ecat_discover;
        if (!ecat_discover.start(SHM_NRT_RX_PDO))
        {
            LOG_ERROR("Failed to connect to EtherCAT PDO shared memory.");
            return 1;
        }
        const auto ecat_map = ecat_discover.discover(config::extract_pdo_ids(*robot));

        zenoh::init_log_from_env_or("error");
        auto session = zenoh::Session::open(make_zenoh_config(options));
        auto config = config::ConfigTopics{{robot->ns, robot->robot_name}};

        auto publishers = std::make_shared<advrf::zenoh_plugin::ZenohAdapterPublishers>();
        if (!publishers->init(config, *robot, ecat_map, session, options.wire_format)) {
            LOG_ERROR("Failed to initialize Zenoh state publishers.");
            return 1;
        }

        auto subscribers = std::make_shared<advrf::zenoh_plugin::ZenohAdapterSubscribers>(
            config, session, options.wire_format);
        //auto service = std::make_shared<advrf::zenoh_plugin::ZenohAdapterService>(topics, session);

        advrf::plugin::PluginExec plugin_exec;
        
        plugin_exec.register_adapter({
            "zenoh_adapter_publishers",
            publishers,
            period_from_rate(options.rate_publishers)});
        plugin_exec.register_adapter({
            "zenoh_adapter_subscribers",
            subscribers,
            period_from_rate(options.rate_subscribers)});
        // plugin_exec.register_adapter({
        //     "zenoh_adapter_service",
        //     service,
        //     period_from_rate(options.rate_service)});

        plugin_exec.start();
        return 0;
    }
    catch (const std::exception& error)
    {
        LOG_ERROR("Zenoh plugin failed: {}", error.what());
        return 1;
    }
}
