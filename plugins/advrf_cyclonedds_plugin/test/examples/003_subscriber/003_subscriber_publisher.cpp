#include <chrono>
#include <filesystem>
#include <thread>
#include <dds/dds.hpp>

#include <advrf_interfaces/msg/MotorTxPdo.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>

#include "advrf_cyclonedds_plugin/config/plugin_config.hpp"
#include "advrf_cyclonedds_plugin/publisher/dds_publisher.hpp"

int main(int argc, char** argv)
{
    advrf::log::Log::init();
    auto plugin_config = advrf::cyclonedds_plugin::parse_plugin_config(
        advrf::middleware::config::ConfigFile::load(advrf::middleware::config::middleware_config_path()));
    advrf::cyclonedds_plugin::validate(plugin_config);
    const auto* config_robot = &plugin_config.common.robot;

    advrf::middleware::config::ConfigTopics topics({config_robot->ns, config_robot->robot_name});
    dds::domain::DomainParticipant participant(plugin_config.dds.domain_id);

    using MotorMsg = advrf_interfaces::msg::dds_::MotorTxPdo_;
    using MotorVectorMsg = advrf_interfaces::msg::dds_::MotorTxPdoVector_;

    advrf::cyclonedds_plugin::DDSPublisher<MotorVectorMsg> publisher;
    publisher.init_dds(
        topics.tx.motorCmd(),
        participant
    );

    while (true) {

        MotorVectorMsg msg;
        MotorMsg motor_1;
        motor_1.ecat_id() = 1;
        motor_1.pos_ref() = 1.0f;
        motor_1.vel_ref() = 2.0f;
        motor_1.tor_ref() = 3.0f;
        motor_1.gain_0() = 0.1f;
        motor_1.gain_1() = 0.2f;
        motor_1.gain_2() = 0.3f;
        motor_1.gain_3() = 0.4f;
        motor_1.gain_4() = 0.5f;
        motor_1.fault_ack() = 0;
        motor_1.ts() = 1;
        motor_1.op_idx_aux() = 0;
        motor_1.aux() = 0.0f;
        msg.data().push_back(motor_1);

        MotorMsg motor_2;
        motor_2.ecat_id() = 2;  
        motor_2.pos_ref() = 10.0f;
        motor_2.vel_ref() = 20.0f;
        motor_2.tor_ref() = 30.0f;
        motor_2.gain_0() = 1.1f;
        motor_2.gain_1() = 1.2f;
        motor_2.gain_2() = 1.3f;
        motor_2.gain_3() = 1.4f;
        motor_2.gain_4() = 1.5f;
        motor_2.fault_ack() = 0;
        motor_2.ts() = 2;
        motor_2.op_idx_aux() = 0;
        motor_2.aux() = 0.0f;
        msg.data().push_back(motor_2);
        publisher.publish(msg);

        std::this_thread::sleep_for(
            std::chrono::milliseconds(100)
        );
    }

    return 0;
}
