#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <zenoh.hxx>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>

namespace
{

using Pdo = iit::advrf::Ec_slave_pdo;

struct Command
{
    std::string key;
    Pdo pdo;
};

Pdo make_pdo(const JointConfig& device, Pdo::Type type)
{
    Pdo pdo;
    pdo.set_type(type);
    pdo.mutable_header()->set_str_id(device.name);
    pdo.mutable_header()->set_index(device.ecat_id);
    return pdo;
}

void add_motor_commands(std::vector<Command>& commands,
                        const config::ConfigTopics& topics,
                        const JointConfig& motor)
{
    auto joint = make_pdo(motor, Pdo::TX_CIA402);
    joint.mutable_cia402_tx_pdo()->set_target_pos(0.0F);
    commands.push_back({topics.command.jointCmd(), std::move(joint)});

    auto legacy = make_pdo(motor, Pdo::TX_MOTOR);
    legacy.mutable_motor_tx_pdo()->set_pos_ref(0.0F);
    commands.push_back({topics.command.motorCmd(), std::move(legacy)});

    auto xt = make_pdo(motor, Pdo::TX_XT_MOTOR);
    xt.mutable_motor_xt_tx_pdo()->set_pos_ref(0.0F);
    commands.push_back({topics.command.motorXtCmd(), std::move(xt)});
}

void add_valve_command(std::vector<Command>& commands,
                       const config::ConfigTopics& topics,
                       const JointConfig& valve)
{
    auto pdo = make_pdo(valve, Pdo::TX_HYQ_KNEE);
    pdo.mutable_hyqknee_tx_pdo()->set_position_ref(0.0F);
    commands.push_back({topics.command.valveCmd(), std::move(pdo)});
}

void add_gripper_command(std::vector<Command>& commands,
                         const config::ConfigTopics& topics,
                         const JointConfig& gripper)
{
    auto pdo = make_pdo(gripper, Pdo::TX_GRIPPER);
    pdo.mutable_gripper_tx_pdo()->set_target_pos(0.0F);
    commands.push_back({topics.command.gripperCmd(), std::move(pdo)});
}

void add_pump_command(std::vector<Command>& commands,
                      const config::ConfigTopics& topics,
                      const JointConfig& pump)
{
    auto pdo = make_pdo(pump, Pdo::TX_HYQ_HPU);
    pdo.mutable_hyqhpu_tx_pdo()->set_pump_target(0.0F);
    commands.push_back({topics.command.pumpCmd(), std::move(pdo)});
}

void add_powerboard_command(std::vector<Command>& commands,
                            const config::ConfigTopics& topics,
                            const JointConfig& power_board)
{
    auto pdo = make_pdo(power_board, Pdo::TX_POW_F28M36);
    pdo.mutable_powf28m36_tx_pdo()->set_master_command(0);
    commands.push_back({topics.command.powerBoardCmd(), std::move(pdo)});
}

void add_forcetorque_command(std::vector<Command>& commands,
                             const config::ConfigTopics& topics,
                             const JointConfig& force_torque)
{
    auto pdo = make_pdo(force_torque, Pdo::TX_FT6);
    pdo.mutable_ft6_tx_pdo()->set_op_idx_aux(0);
    commands.push_back({topics.command.forceTorqueCmd(), std::move(pdo)});
}

bool publish(zenoh::Session& session, Command command)
{
    const auto size = command.pdo.ByteSizeLong();
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        return false;

    std::vector<std::uint8_t> payload(size);
    if (!command.pdo.SerializeToArray(payload.data(), static_cast<int>(size)))
        return false;

    auto publisher = session.declare_publisher(zenoh::KeyExpr(command.key));
    zenoh::Publisher::PutOptions options;
    options.encoding = zenoh::Encoding::Predefined::application_protobuf();
    publisher.put(zenoh::Bytes(std::move(payload)), std::move(options));

    std::cout
        << "Published mock PDO type " << static_cast<int>(command.pdo.type())
        << " for " << command.pdo.header().str_id()
        << " (index " << command.pdo.header().index() << ") on "
        << command.key << '\n';
    return true;
}

}

int main()
{
    try
    {
        const auto config_path = ADVRF_CONFIG_SHARE / "middleware" / "config.yaml";
        const auto robot = load_robot_config(config_path.string());
        if (!robot)
            return 1;

        const config::ConfigTopics topics{{robot->ns, robot->robot_name}};
        std::vector<Command> commands;
        commands.reserve(8);

        if (!robot->motors.empty())
            add_motor_commands(commands, topics, robot->motors.front());
        if (!robot->valves.empty())
            add_valve_command(commands, topics, robot->valves.front());
        if (!robot->grippers.empty())
            add_gripper_command(commands, topics, robot->grippers.front());
        if (!robot->pumps.empty())
            add_pump_command(commands, topics, robot->pumps.front());
        if (!robot->power_boards.empty())
            add_powerboard_command(commands, topics, robot->power_boards.front());
        if (!robot->force_torques.empty())
            add_forcetorque_command(commands, topics, robot->force_torques.front());

        if (commands.empty())
        {
            std::cout << "No transmission-capable devices found in "
                      << config_path << '\n';
            return 0;
        }

        std::cout
            << "WARNING: publishing zero-valued mock commands for "
            << commands.size() << " configured transmission targets.\n";

        auto session = zenoh::Session::open(zenoh::Config::create_default());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        for (auto& command : commands)
        {
            if (!publish(session, std::move(command)))
            {
                std::cerr << "Failed to serialize a mock Protobuf command.\n";
                return 1;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Published " << commands.size()
                  << " mock command messages from " << config_path << '\n';
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Mock command publisher failed: " << error.what() << '\n';
        return 1;
    }
}
