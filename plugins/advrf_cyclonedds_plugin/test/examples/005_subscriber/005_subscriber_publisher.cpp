#include "advrf_cyclonedds_plugin/publisher/dds_publisher.hpp"
#include "advrf_cyclonedds_plugin/config/config_topics.hpp"
#include <advrf_interfaces/msg/ReplCmdContent.hpp>

#include <dds/dds.hpp>

int main(int argc, char** argv)
{
    advrf::log::Log::init();
    dds::domain::DomainParticipant participant(42);
    config::ConfigTopics topics({"advrf", "robot"});

    DDSPublisher<advrf_interfaces::msg::dds_::ReplCmd_Content_Vector_> publisher;
    publisher.init_dds(topics.command.jointCmd(), participant);

    while (true) {
        advrf_interfaces::msg::dds_::ReplCmd_Content_Vector_ msg;
        advrf_interfaces::msg::dds_::ReplCmd_Content_ msg_motor;
        msg_motor.type() = advrf_interfaces::msg::dds_::ReplCmdType::CTRL_CMD;
        msg_motor.trajectory_cmd().type() = advrf_interfaces::msg::dds_::ReplCmdType::TRJ_CMD;
        msg_motor.trajectory_cmd().name() = "motor_1";
        msg_motor.trajectory_cmd().board_id() = 1;
        msg_motor.trajectory_cmd().smooth_par().x().push_back(0.0);
        msg_motor.trajectory_cmd().smooth_par().y().push_back(0.0);
        msg.requests().push_back(msg_motor);

        msg_motor.type() = advrf_interfaces::msg::dds_::ReplCmdType::CTRL_CMD;
        msg_motor.trajectory_cmd().type() = advrf_interfaces::msg::dds_::ReplCmdType::TRJ_CMD;
        msg_motor.trajectory_cmd().name() = "motor_2";
        msg_motor.trajectory_cmd().board_id() = 1;
        msg_motor.trajectory_cmd().smooth_par().x().push_back(0.0);
        msg_motor.trajectory_cmd().smooth_par().y().push_back(0.0);
        msg.requests().push_back(msg_motor);

        publisher.publish(msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}