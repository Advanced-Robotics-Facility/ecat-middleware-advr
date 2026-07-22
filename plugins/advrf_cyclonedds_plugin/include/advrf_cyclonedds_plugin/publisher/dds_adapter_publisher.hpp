#pragma once

#include <array>

#include "advrf_cyclonedds_plugin/publisher/dds_publisher.hpp"
#include <advrf_cyclonedds_plugin/converter.hpp>

template <typename Msg, typename Derived>
class DDSAdapterPublisher
    : public DDSPublisher<Msg, Derived>
    , public middleware_adapter::message::AdapterPublishers::ICallback
{
public:
    using Base = DDSPublisher<Msg, Derived>;
    using Pdo  = iit::advrf::Ec_slave_pdo;

    DDSAdapterPublisher() = default;
    ~DDSAdapterPublisher() override = default;

    bool init(const std::string& robot_name,
              dds::domain::DomainParticipant& participant,
              const std::string& topic_override = "")
    {
        topic_name_ = topic_override.empty()
            ? "rt/advrf/" + robot_name + "/" + std::string{Derived::endpoint}
            : topic_override;

        return Base::init_dds(topic_name_, participant);
    }

    const std::string& topic_name() const
    {
        return topic_name_;
    }

    void on_entry() override{
        message_ = Msg{};
        has_update_ = false;
    }

    void on_pdo(const Pdo& pdo) override
    {
        if(static_cast<Derived*>(this)->consume(pdo)) {
            has_update_ = true;
        }
    }

    void on_exit() override
    {
        if(!has_update_) {return;}

        try {
            this->writer_.write(message_);
        }
        catch (const dds::core::Exception& e) {
            LOG_ERROR("[DDSAdapterPublisher] Write error: {}", e.what());
        }
    }

    virtual bool consume(const Pdo& pdo) = 0;

protected:
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

using Channel = middleware_adapter::message::AdapterPublishers::Channel;

#include <advrf_interfaces/msg/Imu.hpp>
using ImuMsg = ::advrf_interfaces::msg::dds_::Imu_;
class ImuPublisher : public DDSAdapterPublisher<ImuMsg, ImuPublisher> {
public:
    inline static constexpr std::string_view endpoint = "imu";
    inline static std::vector<Channel> channels ={ Channel::Imu };
    
    
    ImuPublisher() = default;

    bool consume(const iit::advrf::Ec_slave_pdo& pdo) override {
        switch (pdo.type()) {
            case iit::advrf::Ec_slave_pdo::RX_IMU_VN:
                convert::dds::from_protobuf(pdo.imuvn_rx_pdo(), message());
                break;
            default:
                LOG_WARN("Unexpected PDO type for ImuPublisher: {}", static_cast<int>(pdo.type()));
                return false; // Exit early if the PDO type is not handled
        }
        std_msgs::msg::dds_::Header_ header;
        convert::dds::from_protobuf(pdo, header);
        message().header() = header;
        return true;
    }
};  

#include <sensor_msgs/msg/JointState.hpp>
using JointStateMsg = ::sensor_msgs::msg::dds_::JointState_;
class JointStatePublisher : public DDSAdapterPublisher<JointStateMsg, JointStatePublisher> {
public:
    inline static constexpr std::string_view endpoint = "joints";
    inline static std::vector<Channel> channels = { Channel::Motor, 
                                                    Channel::Gripper };
    
    JointStatePublisher() = default;
    ~JointStatePublisher() override = default;

    bool consume(const iit::advrf::Ec_slave_pdo& pdo) override {
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
        
        std_msgs::msg::dds_::Header_ header;
        convert::dds::from_protobuf(pdo, header);
        message().header() = header;
      
        return true;
    }
};  


#include <advrf_interfaces/msg/Motor.hpp>
using MotorMsg = ::advrf_interfaces::msg::dds_::Motor_;
class MotorsPublisher : public DDSAdapterPublisher<MotorMsg, MotorsPublisher> {
public:
    inline static constexpr std::string_view endpoint = "motors";
    inline static std::vector<Channel> channels = { Channel::Motor};

    MotorsPublisher() = default;
    ~MotorsPublisher() override = default;

    bool consume(const iit::advrf::Ec_slave_pdo& pdo) override {
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
        
        std_msgs::msg::dds_::Header_ header;
        convert::dds::from_protobuf(pdo, header);
        message().header() = header;
      
        return true;
    }
};  


#include <advrf_interfaces/msg/PowerBoard.hpp>
using PowerBoardMsg = ::advrf_interfaces::msg::dds_::PowerBoard_;
class PowerBoardPublisher : public DDSAdapterPublisher<PowerBoardMsg, PowerBoardPublisher> {
public:
    inline static constexpr std::string_view endpoint = "power_boards";
    inline static std::vector<Channel> channels = { Channel::PowerBoard};

    PowerBoardPublisher() = default;
    ~PowerBoardPublisher() override = default;

    bool consume(const iit::advrf::Ec_slave_pdo& pdo) override {
        switch (pdo.type()) {
            case iit::advrf::Ec_slave_pdo::RX_POW_F28M36:
                convert::dds::from_protobuf(pdo.powf28m36_rx_pdo(), message());
                break;
            default:
                LOG_WARN("Unexpected PDO type for PowerBoardPublisher: {}", static_cast<int>(pdo.type()));
                return false; // Exit early if the PDO type is not handled
        }
        
        std_msgs::msg::dds_::Header_ header;
        convert::dds::from_protobuf(pdo, header);
        message().header() = header;
      
        return true;
    }
};  


#include <advrf_interfaces/msg/Pump.hpp>
using PumpMsg = ::advrf_interfaces::msg::dds_::Pump_;
class PumpPublisher : public DDSAdapterPublisher<PumpMsg, PumpPublisher> {
public:
    inline static constexpr std::string_view endpoint = "pumps";
    inline static std::vector<Channel> channels = { Channel::Pump};

    PumpPublisher() = default;
    ~PumpPublisher() override = default;

    bool consume(const iit::advrf::Ec_slave_pdo& pdo) override {
        switch (pdo.type()) {
            case iit::advrf::Ec_slave_pdo::RX_HYQ_HPU:
                convert::dds::from_protobuf(pdo.hyqhpu_rx_pdo(), message());
                break;
            default:
                LOG_WARN("Unexpected PDO type for PumpPublisher: {}", static_cast<int>(pdo.type()));
                return false; // Exit early if the PDO type is not handled
        }
        
        std_msgs::msg::dds_::Header_ header;
        convert::dds::from_protobuf(pdo, header);
        message().header() = header;
      
        return true;
    }
};  