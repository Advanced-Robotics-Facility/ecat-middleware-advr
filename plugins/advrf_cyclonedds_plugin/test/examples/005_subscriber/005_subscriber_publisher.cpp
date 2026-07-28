#include "advrf_cyclonedds_plugin/publisher/dds_publisher.hpp"
#include <advrf_interfaces/msg/MotorsPdoCmd.hpp>
#include "advrf_cyclonedds_plugin/config/config_topics.hpp"

#include <dds/dds.hpp>

int main(int argc, char** argv)
{
    advrf::log::Log::init();
    dds::domain::DomainParticipant participant(42);
    config::ConfigTopics topics({"advrf", "robot"});

    DDSPublisher<advrf_interfaces::msg::dds_::MotorsPdoCmd_> publisher;
    publisher.init_dds(topics.command.jointCmd(), participant);

    while (true) {
        advrf_interfaces::msg::dds_::MotorsPdoCmd_ msg;
        advrf_interfaces::msg::dds_::MotorPdoCmd_ msg_motor;
        msg_motor.motor_id() = 1;
        msg_motor.pos_ref() = 0.5;
        msg_motor.vel_ref() = 0.1;
        msg_motor.tor_ref() = 0.05;
        msg.motors_pdo().push_back(msg_motor);

        msg_motor.motor_id() = 2;
        msg_motor.pos_ref() = 0.1;
        msg_motor.vel_ref() = 0.0;
        msg_motor.tor_ref() = 0.85;
        msg.motors_pdo().push_back(msg_motor);

        publisher.publish(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}