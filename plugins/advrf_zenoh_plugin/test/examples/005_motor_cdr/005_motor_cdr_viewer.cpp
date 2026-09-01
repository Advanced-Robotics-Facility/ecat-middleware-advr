#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <zenoh.hxx>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>

#include "advrf_zenoh_plugin/serialization/ros2cdr.hpp"

namespace
{

using Pdo = iit::advrf::Ec_slave_pdo;
using Decoder = advrf::zenoh_plugin::deserialization::Ros2CdrDeserializer;
using CommandType = advrf::zenoh_plugin::deserialization::Ros2CommandType;

volatile std::sig_atomic_t running = 1;

void stop(int)
{
    running = 0;
}

struct Options
{
    std::string key;
    bool help{false};
};

Options parse_options(int argc, char** argv)
{
    Options options;

    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--key")
        {
            if (++index >= argc)
                throw std::runtime_error("Missing value after --key");
            options.key = argv[index];
        }
        else if (argument == "-h" || argument == "--help")
        {
            options.help = true;
        }
        else
        {
            throw std::runtime_error("Unknown argument: " + argument);
        }
    }

    return options;
}

void print_help(const char* program)
{
    std::cout
        << "Usage:\n"
        << "  " << program << " [--key ZENOH_KEY]\n\n"
        << "Decode ROS 2 MotorTxPdoVector CDR samples with the Zenoh plugin's\n"
        << "motor-command deserializer and print the resulting middleware PDOs.\n\n"
        << "If --key is omitted, the motor command key is read from the installed\n"
        << "robot configuration.\n";
}

std::string configured_motor_key()
{
    const auto id_map_path = ADVRF_CONFIG_SHARE / "robot_id_map" / "robot_id_map.yaml";
    const auto ecat_config_path = ADVRF_CONFIG_SHARE / "robot_ecat" / "ecat_config.yaml";
    const auto robot = advrf::middleware::config::load_robot_config(id_map_path.string(), ecat_config_path.string());
    if (!robot)
        throw std::runtime_error(
            "Unable to load robot configuration from " + ecat_config_path.string());

    const advrf::middleware::config::ConfigTopics topics{{robot->ns, robot->robot_name}};
    return topics.tx.motorCmd();
}

void print_motor(const Pdo& pdo)
{
    if (pdo.type() != Pdo::TX_MOTOR || !pdo.has_motor_tx_pdo())
    {
        std::cerr << "Decoded an unexpected PDO type "
                  << static_cast<int>(pdo.type()) << '\n';
        return;
    }

    const auto& motor = pdo.motor_tx_pdo();
    std::cout
        << "middleware motor PDO"
        << " ecat_id=" << pdo.header().index()
        << " pos_ref=" << motor.pos_ref()
        << " gainp=" << motor.gainp()
        << " gaind=" << motor.gaind()
        << " fault_ack=" << motor.fault_ack()
        << " ts=" << motor.ts()
        << '\n';
}

}

int main(int argc, char** argv)
{
    using namespace std::chrono_literals;

    try
    {
        const auto options = parse_options(argc, argv);
        if (options.help)
        {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }

        const std::string key = options.key.empty()
            ? configured_motor_key()
            : options.key;

        std::signal(SIGINT, stop);
        std::signal(SIGTERM, stop);

        auto session = zenoh::Session::open(zenoh::Config::create_default());
        const Decoder decoder(CommandType::Motor);
        auto subscriber = session.declare_subscriber(
            zenoh::KeyExpr(key),
            [decoder](const zenoh::Sample& sample)
            {
                const auto payload = sample.get_payload().as_vector();
                std::vector<Pdo> commands;
                if (!decoder.deserialize_cycle(payload, commands))
                {
                    std::cerr
                        << "Could not decode sample as a ROS 2 "
                        << "MotorTxPdoVector CDR payload\n";
                    return;
                }

                std::cout << "received CDR sample: " << payload.size()
                          << " bytes, " << commands.size() << " motor(s)\n";
                for (const auto& command : commands)
                    print_motor(command);
            },
            zenoh::closures::none);

        std::cout << "Viewing ROS 2 CDR motor commands on Zenoh key '"
                  << key << "'. Press Ctrl-C to stop.\n";

        while (running)
            std::this_thread::sleep_for(200ms);

        return EXIT_SUCCESS;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Motor CDR viewer failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
