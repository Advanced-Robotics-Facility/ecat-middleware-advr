#pragma once

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include "advrf_cyclonedds_plugin/publisher/dds_publisher.hpp"
#include <advrf_dds_common/converter/converter.hpp>

#include "advrf_cyclonedds_plugin/ros_metadata/ros_graph_bridge.hpp"

template <typename Msg>
class DDSAdapterBridgePublisher
    : public DDSPublisher<Msg>,
      public IConnectRosGraphBridge,
      public middleware_adapter::message::AdapterPublishers::IPublisher
{
public:
    using Pdo  = iit::advrf::Ec_slave_pdo;

    DDSAdapterBridgePublisher() = default;
    ~DDSAdapterBridgePublisher() override = default;

    bool init(const std::string& topic_name,
              dds::domain::DomainParticipant& participant)
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

        try {
            this->writer_.write(message_);
        }
        catch (const dds::core::Exception& e) {
            LOG_ERROR("[DDSAdapterPublisher] Write error: {}", e.what());
        }
    }

    void connect_ros_graph_bridge(CycloneDDSRosGraphBridge &bridge) override {
        bridge.add_writer(this->writer_);
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


#include <advrf_interfaces/msg/Imu.hpp>
#include <sensor_msgs/msg/JointState.hpp>
#include <advrf_interfaces/msg/Motor.hpp>
#include <advrf_interfaces/msg/PowerBoard.hpp>
#include <advrf_interfaces/msg/Pump.hpp>
#include <advrf_interfaces/msg/ForceTorque.hpp>
#include <advrf_interfaces/msg/Valve.hpp>
#include <advrf_interfaces/msg/Gripper.hpp>

using ImuMsg = ::advrf_interfaces::msg::dds_::Imu_;
using JointStateMsg = ::sensor_msgs::msg::dds_::JointState_;
using MotorMsg = ::advrf_interfaces::msg::dds_::Motor_;
using PowerBoardMsg = ::advrf_interfaces::msg::dds_::PowerBoard_;
using PumpMsg = ::advrf_interfaces::msg::dds_::Pump_;
using ForceTorqueMsg = ::advrf_interfaces::msg::dds_::ForceTorque_;
using ValveMsg = ::advrf_interfaces::msg::dds_::Valve_;
using GripperMsg = ::advrf_interfaces::msg::dds_::Gripper_;

class ImuPublisher : public DDSAdapterBridgePublisher<ImuMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class JointStatePublisher : public DDSAdapterBridgePublisher<JointStateMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class MotorsPublisher : public DDSAdapterBridgePublisher<MotorMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class PowerBoardPublisher : public DDSAdapterBridgePublisher<PowerBoardMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class PumpPublisher : public DDSAdapterBridgePublisher<PumpMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class ForceTorquePublisher : public DDSAdapterBridgePublisher<ForceTorqueMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class ValvePublisher : public DDSAdapterBridgePublisher<ValveMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

class GripperPublisher : public DDSAdapterBridgePublisher<GripperMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  
