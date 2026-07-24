#pragma once

#include "advrf_cyclonedds_plugin/publisher/dds_publisher.hpp"
#include <advrf_cyclonedds_plugin/converter.hpp>

template <typename Msg>
class DDSAdapterBridgePublisher
    : public DDSPublisher<Msg, DDSAdapterBridgePublisher<Msg>>
    , public middleware_adapter::message::AdapterPublishers::IPublisher
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
using ImuMsg = ::advrf_interfaces::msg::dds_::Imu_;
class ImuPublisher : public DDSAdapterBridgePublisher<ImuMsg> {

protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override {
        switch (pdo.type()) {
            case iit::advrf::Ec_slave_pdo::RX_IMU_VN:
                convert::dds::from_protobuf(pdo.imuvn_rx_pdo(), message());
                break;
            default:
                LOG_WARN("Unexpected PDO type for ImuPublisher: {}", static_cast<int>(pdo.type()));
                return false; // Exit early if the PDO type is not handled
        }
        convert::dds::from_protobuf(pdo, message().header());
        return true;
    }
};  

#include <sensor_msgs/msg/JointState.hpp>
using JointStateMsg = ::sensor_msgs::msg::dds_::JointState_;
class JointStatePublisher : public DDSAdapterBridgePublisher<JointStateMsg> {


protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override {
        switch (pdo.type()) {
            case iit::advrf::Ec_slave_pdo::RX_CIA402:
                convert::dds::from_protobuf(pdo.cia402_rx_pdo(), message());
                break;
            case iit::advrf::Ec_slave_pdo::RX_XT_MOTOR:
                convert::dds::from_protobuf(pdo.motor_xt_rx_pdo(), message());
                break;
            case iit::advrf::Ec_slave_pdo::RX_MOTOR:
                convert::dds::from_protobuf(pdo.motor_rx_pdo(), message());
                break;
            case iit::advrf::Ec_slave_pdo::RX_GRIPPER:
                convert::dds::from_protobuf(pdo.gripper_rx_pdo(), message());
                break;
            default:
                LOG_WARN("Unexpected PDO type for JointStatePublisher: {}", static_cast<int>(pdo.type()));
                return false; // Exit early if the PDO type is not handled
        }
        
        convert::dds::from_protobuf(pdo, message().header());
        return true;
    }
};  


#include <advrf_interfaces/msg/Motor.hpp>
using MotorMsg = ::advrf_interfaces::msg::dds_::Motor_;
class MotorsPublisher : public DDSAdapterBridgePublisher<MotorMsg> {


protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override {
        switch (pdo.type()) {
            case iit::advrf::Ec_slave_pdo::RX_CIA402:
                convert::dds::from_protobuf(pdo.cia402_rx_pdo(), message());
                break;
            case iit::advrf::Ec_slave_pdo::RX_XT_MOTOR:
                convert::dds::from_protobuf(pdo.motor_xt_rx_pdo(), message());
                break;
            case iit::advrf::Ec_slave_pdo::RX_MOTOR:
                convert::dds::from_protobuf(pdo.motor_rx_pdo(), message());
                break;
            default:
                LOG_WARN("Unexpected PDO type for MotorsPublisher: {}", static_cast<int>(pdo.type()));
                return false; // Exit early if the PDO type is not handled
        }
        
        convert::dds::from_protobuf(pdo, message().header());
        return true;
    }
};  


#include <advrf_interfaces/msg/PowerBoard.hpp>
using PowerBoardMsg = ::advrf_interfaces::msg::dds_::PowerBoard_;
class PowerBoardPublisher : public DDSAdapterBridgePublisher<PowerBoardMsg> {


protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override {
        switch (pdo.type()) {
            case iit::advrf::Ec_slave_pdo::RX_POW_F28M36:
                convert::dds::from_protobuf(pdo.powf28m36_rx_pdo(), message());
                break;
            default:
                LOG_WARN("Unexpected PDO type for PowerBoardPublisher: {}", static_cast<int>(pdo.type()));
                return false; // Exit early if the PDO type is not handled
        }
        
        convert::dds::from_protobuf(pdo, message().header());
        return true;
    }
};  


#include <advrf_interfaces/msg/Pump.hpp>
using PumpMsg = ::advrf_interfaces::msg::dds_::Pump_;
class PumpPublisher : public DDSAdapterBridgePublisher<PumpMsg> {


protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override {
        switch (pdo.type()) {
            case iit::advrf::Ec_slave_pdo::RX_HYQ_HPU:
                convert::dds::from_protobuf(pdo.hyqhpu_rx_pdo(), message());
                break;
            default:
                LOG_WARN("Unexpected PDO type for PumpPublisher: {}", static_cast<int>(pdo.type()));
                return false; // Exit early if the PDO type is not handled
        }
        
        convert::dds::from_protobuf(pdo, message().header());
        return true;
    }
};  


#include <advrf_interfaces/msg/ForceTorque.hpp>
using ForceTorqueMsg = ::advrf_interfaces::msg::dds_::ForceTorque_;
class ForceTorquePublisher : public DDSAdapterBridgePublisher<ForceTorqueMsg> {

protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override {
        switch (pdo.type()) {
            case iit::advrf::Ec_slave_pdo::RX_FT6:
                convert::dds::from_protobuf(pdo.ft6_rx_pdo(), message());
                break;
            default:
                LOG_WARN("Unexpected PDO type for ForceTorquePublisher: {}", static_cast<int>(pdo.type()));
                return false; // Exit early if the PDO type is not handled
        }
        
        convert::dds::from_protobuf(pdo, message().header());
        return true;
    }
};  