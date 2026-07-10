#pragma once

#include <iostream>
#include <atomic>
#include <string>
#include <vector>
#include "msg.hpp"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

/* Includes fastddsgen generated data types. */
#include "JointStatePubSubTypes.hpp"
#include "ImuPubSubTypes.hpp"
#include "ForceTorquePubSubTypes.hpp"
#include "MotorPubSubTypes.hpp"
#include "PowerBoardPubSubTypes.hpp"
#include "PumpPubSubTypes.hpp"
#include "ValvePubSubTypes.hpp"
#include "GripperPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

using JointStateMsg  = ::sensor_msgs::msg::dds_::JointState_;
using ImuMsg         = ::advrf_msgs::msg::dds_::Imu_;
using ForceTorqueMsg = ::advrf_msgs::msg::dds_::ForceTorque_;
using MotorMsg       = ::advrf_msgs::msg::dds_::Motor_;
using PowerBoardMsg  = ::advrf_msgs::msg::dds_::PowerBoard_;
using PumpMsg        = ::advrf_msgs::msg::dds_::Pump_;
using ValveMsg       = ::advrf_msgs::msg::dds_::Valve_;
using GripperMsg     = ::advrf_msgs::msg::dds_::Gripper_;

////////////////////////////
//  Base Class Publisher  //
////////////////////////////

template <typename Msg, typename Derived>
class DdsPublisher {
    public:

        explicit DdsPublisher(TypeSupport type)
            : participant_(nullptr), 
              publisher_(nullptr), 
              topic_(nullptr), 
              writer_(nullptr), 
              type_(std::move(type)), 
              listener_(*this) 
        {}

        virtual ~DdsPublisher() { 
            cleanup(); 
        }

        bool has_subscribers() const { 
            return listener_.matched_count() > 0; 
        }

    protected:

        bool init_dds(const std::string& topic_name, DomainParticipant* participant) {
            if (!participant) 
                return false;
            participant_ = participant;

            // Trick for ROS2 Compatibility
            std::string type_name = type_.get_type_name();

            type_.register_type(participant_);
            
            topic_ = participant_->create_topic(topic_name, type_.get_type_name(), TOPIC_QOS_DEFAULT);
            
            publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
            
            DataWriterQos qos = static_cast<Derived*>(this)->get_writer_qos();
            
            writer_ = publisher_->create_datawriter(topic_, qos, &listener_);
            
            return (writer_ != nullptr);
        }

        DataWriterQos get_writer_qos() {
            DataWriterQos qos;
            qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
            qos.history().kind = KEEP_LAST_HISTORY_QOS;
            qos.history().depth = 1;
            return qos;
        }

        template <typename T>
        void set_timestamp(T& stamp, uint64_t ts_ns) {
            stamp.sec() = static_cast<int32_t>(ts_ns / 1'000'000'000ULL);
            stamp.nanosec() = static_cast<uint32_t>(ts_ns % 1'000'000'000ULL);
        }

        void cleanup() {
            if (participant_ && writer_) 
                publisher_->delete_datawriter(writer_);
            
            if (participant_ && publisher_) 
                participant_->delete_publisher(publisher_);
            
            if (participant_ && topic_) 
                participant_->delete_topic(topic_);
        }

        DomainParticipant* participant_;
        Publisher* publisher_;
        Topic* topic_;
        DataWriter* writer_;
        TypeSupport type_;

    private:

        class PubListener : public DataWriterListener {
            public:
                
                explicit PubListener(DdsPublisher& p) 
                    : parent_(p), matched_{0} 
                {}

                void on_publication_matched(DataWriter*, 
                                            const PublicationMatchedStatus& info) override {
                    matched_ += info.current_count_change;
                }

                int matched_count() const { 
                    return matched_.load(); 
                }

            private:

                DdsPublisher& parent_;
                std::atomic_int matched_;

        } listener_;
};

///////////////////////////
// Joint State Publisher //
///////////////////////////

class JointStatePublisher : public DdsPublisher<JointStateMsg, JointStatePublisher> {
    public:

        JointStatePublisher() 
            : DdsPublisher(TypeSupport{new sensor_msgs::msg::dds_::JointState_PubSubType()}) 
        {}

        bool init(const std::vector<std::string>& names, 
                  const std::string& robot_name, 
                  DomainParticipant* participant) {
            
            if (!init_dds("rt/advrf/" + robot_name + "/joint_states", participant)) 
                return false;

            joint_state_.header().frame_id("");
            joint_state_.name() = names;
            joint_state_.position().assign(names.size(), 0.0);
            joint_state_.velocity().assign(names.size(), 0.0);
            joint_state_.effort().assign(names.size(), 0.0);
            
            return true;
        }

        void publish(const joint_state::rt_joint_state_msg& msg) {
            
            if (!has_subscribers()) 
                return;
             
            set_timestamp(joint_state_.header().stamp(), msg.header.timestamp_ns);
            joint_state_.position() = msg.positions;
            joint_state_.velocity() = msg.velocities;
            joint_state_.effort() = msg.efforts;

            writer_->write(&joint_state_);
        }

    private:

        JointStateMsg joint_state_;
};

///////////////////////////
//     Imu Publisher     //
///////////////////////////

class ImuPublisher : public DdsPublisher<ImuMsg, ImuPublisher> {
    public:
        ImuPublisher() 
            : DdsPublisher(TypeSupport{new advrf_msgs::msg::dds_::Imu_PubSubType()}) 
        {}

        bool init(const std::string& robot_name, 
                DomainParticipant* participant, 
                const std::string& topic_override = "") {

            imu_.header().frame_id() = "";

            return init_dds(topic_override.empty() 
                ? "rt/advrf/" + robot_name + "/imu" 
                : topic_override, participant);
        }

        void publish(const imu::rt_imu_msg& msg) {

            if (!has_subscribers()) 
                return;

            set_timestamp(imu_.header().stamp(), msg.header.timestamp_ns);

            imu_.linear_acceleration().x()  = msg.linear_acceleration.x;
            imu_.linear_acceleration().y()  = msg.linear_acceleration.y;
            imu_.linear_acceleration().z()  = msg.linear_acceleration.z;
            
            imu_.angular_velocity().x()     = msg.angular_velocity.x;
            imu_.angular_velocity().y()     = msg.angular_velocity.y;
            imu_.angular_velocity().z()     = msg.angular_velocity.z;
            
            imu_.orientation().x()          = msg.orientation.x;
            imu_.orientation().y()          = msg.orientation.y;
            imu_.orientation().z()          = msg.orientation.z;
            imu_.orientation().w()          = msg.orientation.w;
            
            imu_.imu_ts()                   = msg.imu_ts;
            imu_.temperature()              = msg.temperature;
            imu_.digital_in()               = msg.digital_in;
            imu_.fault()                    = msg.fault;
            imu_.rtt()                      = msg.rtt;

            writer_->write(&imu_);
        }

    private:

        ImuMsg imu_;
};

////////////////////////////
// Force Torque Publisher //
////////////////////////////

class ForceTorquePublisher : public DdsPublisher<ForceTorqueMsg, ForceTorquePublisher> {
    public:
        ForceTorquePublisher() 
            : DdsPublisher(TypeSupport{new advrf_msgs::msg::dds_::ForceTorque_PubSubType()}) 
        {}

        bool init(const std::string& robot_name, 
                  DomainParticipant* participant, 
                  const std::string& topic_override = "") {

            ft_.header().frame_id() = "";

            return init_dds(topic_override.empty() 
                ? "rt/advrf/" + robot_name + "/force_torque" 
                : topic_override, participant);
        }

        void publish(const force_torque::rt_force_torque_msg& msg) {

            if (!has_subscribers()) 
                return;

            set_timestamp(ft_.header().stamp(), msg.header.timestamp_ns);

            ft_.wrench().force().x()       = msg.wrench.force.x;
            ft_.wrench().force().y()       = msg.wrench.force.y;
            ft_.wrench().force().z()       = msg.wrench.force.z;
            
            ft_.wrench().torque().x()      = msg.wrench.torque.x;
            ft_.wrench().torque().y()      = msg.wrench.torque.y;
            ft_.wrench().torque().z()      = msg.wrench.torque.z;
        
            ft_.fault()                    = msg.fault;
            ft_.rtt()                      = msg.rtt;
            ft_.op_idx_ack()               = msg.op_idx_ack;
            ft_.aux()                      = msg.aux;

            writer_->write(&ft_);
        }

    private:

        ForceTorqueMsg ft_;
};

////////////////////////////
//     Motor Publisher    //
////////////////////////////

class MotorPublisher : public DdsPublisher<MotorMsg, MotorPublisher> {
    public:

        MotorPublisher() 
            : DdsPublisher(TypeSupport{new advrf_msgs::msg::dds_::Motor_PubSubType()}) 
        {}

        bool init(const std::vector<std::string>& motor_names, 
                  const std::string& robot_name, 
                  DomainParticipant* participant) {

            if (!init_dds("rt/advrf/" + robot_name + "/motor", participant)) 
                return false;

            const size_t n = motor_names.size();
            motor_.name() = motor_names;
            motor_.statusword().assign(n, 0);
            motor_.modes_of_op().assign(n, 0);
            motor_.motor_pos().assign(n, 0.0f);
            motor_.motor_vel().assign(n, 0.0f);
            motor_.link_pos().assign(n, 0.0f);
            motor_.link_vel().assign(n, 0.0f);
            motor_.current().assign(n, 0.0f);
            motor_.torque().assign(n, 0.0f);
            motor_.demanded_pos().assign(n, 0.0f);
            motor_.demanded_vel().assign(n, 0.0f);
            motor_.demanded_torque().assign(n, 0.0f);
            motor_.demanded_current().assign(n, 0.0f);
            motor_.control_effort().assign(n, 0.0f);
            motor_.motor_temp().assign(n, 0.0f);
            motor_.drive_temp().assign(n, 0);
            motor_.error_code().assign(n, 0);
            motor_.error_report().assign(n, "");
            motor_.fault().assign(n, 0);
            motor_.rtt().assign(n, 0);
            motor_.header().frame_id() = "";
            
            return true;
        }

        void publish(const motor::rt_motor_msg& msg) {

            if (!has_subscribers()) 
                return;

            set_timestamp(motor_.header().stamp(), msg.header.timestamp_ns);

            motor_.statusword()       = msg.statusword;
            motor_.modes_of_op()      = msg.modes_of_op;
            motor_.motor_pos()        = msg.motor_pos;
            motor_.motor_vel()        = msg.motor_vel;
            motor_.link_pos()         = msg.link_pos;
            motor_.link_vel()         = msg.link_vel;
            motor_.current()          = msg.current;
            motor_.torque()           = msg.torque;
            motor_.demanded_pos()     = msg.demanded_pos;
            motor_.demanded_vel()     = msg.demanded_vel;
            motor_.demanded_torque()  = msg.demanded_torque;
            motor_.demanded_current() = msg.demanded_current;
            motor_.control_effort()   = msg.control_effort;
            motor_.motor_temp()       = msg.motor_temp;
            motor_.drive_temp()       = msg.drive_temp;
            motor_.error_code()       = msg.error_code;
            motor_.fault()            = msg.fault;
            motor_.rtt()              = msg.rtt;

            writer_->write(&motor_);
        }

    private:

        MotorMsg motor_;
};

///////////////////////////
// Power Board Publisher //
///////////////////////////

class PowerBoardPublisher : public DdsPublisher<PowerBoardMsg, PowerBoardPublisher> {
    public:
        PowerBoardPublisher() 
            : DdsPublisher(TypeSupport{new advrf_msgs::msg::dds_::PowerBoard_PubSubType()}) 
        {}

        bool init(const std::string& robot_name, 
                  DomainParticipant* participant, 
                  const std::string& topic_override = "") {

            pb_.header().frame_id() = "";

            return init_dds(topic_override.empty() 
                ? "rt/advrf/" + robot_name + "/power_board" 
                : topic_override, participant);
        }
        void publish(const power_board::rt_power_board_msg& msg) {

            if (!has_subscribers()) 
                return;

            set_timestamp(pb_.header().stamp(), msg.header.timestamp_ns);

            pb_.v_batt()        = msg.v_batt;
            pb_.v_load()        = msg.v_load;
            pb_.i_load()        = msg.i_load;

            pb_.temperature()   = msg.temperature;
            pb_.temp_batt()     = msg.temp_batt;
            pb_.temp_heatsink() = msg.temp_heatsink;
            
            pb_.status()        = msg.status;
            pb_.fault()         = msg.fault;
            pb_.rtt()           = msg.rtt;
            pb_.op_idx_ack()    = msg.op_idx_ack;
            pb_.aux()           = msg.aux;

            writer_->write(&pb_);
        }

    private:

        PowerBoardMsg pb_;
};

///////////////////////////
//     Pump Publisher    //
///////////////////////////

class PumpPublisher : public DdsPublisher<PumpMsg, PumpPublisher> {
    public:
        PumpPublisher() 
            : DdsPublisher(TypeSupport{new advrf_msgs::msg::dds_::Pump_PubSubType()}) 
        {}

        bool init(const std::vector<std::string>& pump_names, 
                const std::string& robot_name, 
                DomainParticipant* participant) {

            if (!init_dds("rt/advrf/" + robot_name + "/pump", participant)) 
                return false;

            const size_t n = pump_names.size();
            pump_.name() = pump_names;
            pump_.motor_current().assign(n, 0.0f);
            pump_.motor_speed().assign(n, 0.0f);
            pump_.pressure1().assign(n, 0.0f);
            pump_.pressure2().assign(n, 0.0f);
            pump_.temperature().assign(n, 0);
            pump_.mosfet_temperature().assign(n, 0);
            pump_.motor_temperature().assign(n, 0);
            pump_.fault().assign(n, 0);
            pump_.rtt().assign(n, 0);
            pump_.op_idx_ack().assign(n, 0);
            pump_.aux().assign(n, 0.0f);
            pump_.header().frame_id() = "";

            return true;
        }

        void publish(const pump::rt_pump_msg& msg) {

            if (!has_subscribers()) 
                return;
            
            set_timestamp(pump_.header().stamp(), msg.header.timestamp_ns);

            pump_.motor_current()            = msg.motor_current;
            pump_.motor_speed()              = msg.motor_speed;

            pump_.pressure1()                = msg.pressure1;
            pump_.pressure2()                = msg.pressure2;

            pump_.temperature()              = msg.temperature;
            pump_.mosfet_temperature()       = msg.mosfet_temperature;
            pump_.motor_temperature()        = msg.motor_temperature;
            
            pump_.fault()                    = msg.fault;
            pump_.rtt()                      = msg.rtt;
            pump_.op_idx_ack()               = msg.op_idx_ack;
            pump_.aux()                      = msg.aux;

            writer_->write(&pump_);
        }

    private:

        PumpMsg pump_;
};

////////////////////////////
//     Valve Publisher    //
////////////////////////////

class ValvePublisher : public DdsPublisher<ValveMsg, ValvePublisher> {
    public:
        ValvePublisher() 
            : DdsPublisher(TypeSupport{new advrf_msgs::msg::dds_::Valve_PubSubType()}) 
        {}

        bool init(const std::vector<std::string>& valve_names, 
                  const std::string& robot_name, 
                  DomainParticipant* participant) {

            if (!init_dds("rt/advrf/" + robot_name + "/valve", participant)) 
                return false;
            
            const size_t n = valve_names.size();
            valve_.name() = valve_names;
            valve_.encoder_position().assign(n, 0.0f);
            valve_.force().assign(n, 0.0f);
            valve_.pressure1().assign(n, 0.0f);
            valve_.pressure2().assign(n, 0.0f);
            valve_.current().assign(n, 0.0f);
            valve_.temperature().assign(n, 0.0f);
            valve_.current_ref_fb().assign(n, 0.0f);
            valve_.position_ref_fb().assign(n, 0.0f);
            valve_.force_ref_fb().assign(n, 0.0f);
            valve_.fault().assign(n, 0);
            valve_.rtt().assign(n, 0);
            valve_.op_idx_ack().assign(n, 0);
            valve_.aux().assign(n, 0.0f);
            valve_.header().frame_id() = "";

            return true;
        }

        void publish(const valve::rt_valve_msg& msg) {

            if (!has_subscribers()) 
                return;

            set_timestamp(valve_.header().stamp(), msg.header.timestamp_ns);

            valve_.encoder_position() = msg.encoder_position;
            valve_.force()            = msg.force;
            valve_.pressure1()        = msg.pressure1;
            valve_.pressure2()        = msg.pressure2;
            valve_.current()          = msg.current;
            valve_.temperature()      = msg.temperature;
            valve_.current_ref_fb()   = msg.current_ref_fb;
            valve_.position_ref_fb()  = msg.position_ref_fb;
            valve_.force_ref_fb()     = msg.force_ref_fb;
            valve_.fault()            = msg.fault;
            valve_.rtt()              = msg.rtt;
            valve_.op_idx_ack()       = msg.op_idx_ack;
            valve_.aux()              = msg.aux;

            writer_->write(&valve_);
        }

    private:

        ValveMsg valve_;
};