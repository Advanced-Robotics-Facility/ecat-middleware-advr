#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>

#include <advrf_fastdds_plugin/publisher/dds_publisher.hpp>
#include <advrf_dds_common/converter/converter.hpp>

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_middleware_core/utils/log.hpp>
#include "advrf_fastdds_plugin/ros_metadata/ros_graph_bridge.hpp"

template <typename Msg, typename MsgPubSubType>
class DDSAdapterBridgePublisher
    : public DDSPublisher<Msg, MsgPubSubType>
    , public middleware_adapter::message::AdapterPublishers::IPublisher,
    public IConnectRosGraphBridge
{
public:
    using Pdo  = iit::advrf::Ec_slave_pdo;

    DDSAdapterBridgePublisher() = default;
    ~DDSAdapterBridgePublisher() override = default;

    bool init(const std::string& topic_name,
              eprosima::fastdds::dds::DomainParticipant* participant)
    {
        topic_name_ = topic_name;
        return this->init_dds(topic_name_, participant);
    }

    const std::string& topic_name() const
    {
        return topic_name_;
    }

    void begin_cycle() override{
        message_ = Msg{};
        has_update_ = false;
    }

    void consume(const Pdo& pdo) override
    {
        if(process(pdo)) {
            has_update_ = true;
        }
    }

    void end_cycle(bool valid) override
    {
        if(!valid || !has_update_) {return;}

        if (this->writer_ == nullptr) {
            LOG_ERROR("[DDSAdapterBridges] Write error: writer not initialized");
            return;
        }
        eprosima::fastdds::dds::ReturnCode_t ret = this->writer_->write(&message_);
        if (ret != eprosima::fastdds::dds::RETCODE_OK) {
            LOG_ERROR("[DDSAdapterBridges] write() retcode: {}", ret);            
        }
    }

    void connect_ros_graph_bridge(FastRosGraphBridge &bridge) override {
        bridge.add_writer(this->dds_writer());
    }

protected:

    virtual bool process(const Pdo& pdo) = 0;

    Msg& message(){
        return message_;
    }

    const Msg& message() const {
        return message_;
    }


private:
    Msg message_;
    std::string topic_name_;
    bool has_update_ = false;
};


#include <advrf_interfaces/msg/ImuPubSubTypes.hpp>
#include <sensor_msgs/msg/JointStatePubSubTypes.hpp>
#include <advrf_interfaces/msg/MotorPubSubTypes.hpp>
#include <advrf_interfaces/msg/PowerBoardPubSubTypes.hpp>
#include <advrf_interfaces/msg/PumpPubSubTypes.hpp>
#include <advrf_interfaces/msg/ForceTorquePubSubTypes.hpp>
#include <advrf_interfaces/msg/ValvePubSubTypes.hpp>
#include <advrf_interfaces/msg/GripperPubSubTypes.hpp>

using ImuMsg = ::advrf_interfaces::msg::dds_::Imu_;
using JointStateMsg = ::sensor_msgs::msg::dds_::JointState_;
using MotorMsg = ::advrf_interfaces::msg::dds_::Motor_;
using PowerBoardMsg = ::advrf_interfaces::msg::dds_::PowerBoard_;
using PumpMsg = ::advrf_interfaces::msg::dds_::Pump_;
using ForceTorqueMsg = ::advrf_interfaces::msg::dds_::ForceTorque_;
using ValveMsg = ::advrf_interfaces::msg::dds_::Valve_;
using GripperMsg = ::advrf_interfaces::msg::dds_::Gripper_;

using ImuMsgPubSubType = ::advrf_interfaces::msg::dds_::Imu_PubSubType;
using JointStateMsgPubSubType = ::sensor_msgs::msg::dds_::JointState_PubSubType;
using MotorMsgPubSubType = ::advrf_interfaces::msg::dds_::Motor_PubSubType;
using PowerBoardMsgPubSubType = ::advrf_interfaces::msg::dds_::PowerBoard_PubSubType;
using PumpMsgPubSubType = ::advrf_interfaces::msg::dds_::Pump_PubSubType;
using ForceTorqueMsgPubSubType = ::advrf_interfaces::msg::dds_::ForceTorque_PubSubType;
using ValveMsgPubSubType = ::advrf_interfaces::msg::dds_::Valve_PubSubType;
using GripperMsgPubSubType = ::advrf_interfaces::msg::dds_::Gripper_PubSubType;

class ImuPublisher : public DDSAdapterBridgePublisher<ImuMsg, ImuMsgPubSubType> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class JointStatePublisher : public DDSAdapterBridgePublisher<JointStateMsg, JointStateMsgPubSubType> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class MotorsPublisher : public DDSAdapterBridgePublisher<MotorMsg, MotorMsgPubSubType> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class PowerBoardPublisher : public DDSAdapterBridgePublisher<PowerBoardMsg, PowerBoardMsgPubSubType> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class PumpPublisher : public DDSAdapterBridgePublisher<PumpMsg, PumpMsgPubSubType> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class ForceTorquePublisher : public DDSAdapterBridgePublisher<ForceTorqueMsg, ForceTorqueMsgPubSubType> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class ValvePublisher : public DDSAdapterBridgePublisher<ValveMsg, ValveMsgPubSubType> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class GripperPublisher : public DDSAdapterBridgePublisher<GripperMsg, GripperMsgPubSubType> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  
