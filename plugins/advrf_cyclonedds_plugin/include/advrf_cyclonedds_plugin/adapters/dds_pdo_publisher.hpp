#pragma once

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include "advrf_cyclonedds_plugin/publisher/dds_publisher.hpp"
#include <advrf_dds_common/converter/converter.hpp>

#include "advrf_cyclonedds_plugin/ros_metadata/ros_graph_bridge.hpp"

namespace advrf::cyclonedds_plugin {

/**
 * @brief Base class that converts EtherCAT PDOs into one DDS message per cycle.
 *
 * Derived classes implement @ref process to merge compatible PDOs into the
 * current message. The message is published only when the adapter reports a
 * valid cycle and at least one PDO updated it.
 *
 * @tparam Msg DDS message type published by this object.
 */
template <typename Msg>
class DDSPdoPublisher
    : public DDSPublisher<Msg>,
      public IConnectRosGraphBridge,
      public middleware_adapter::message::AdapterPublishers::IPublisher
{
public:
    using Pdo  = iit::advrf::Ec_slave_pdo;

    DDSPdoPublisher() = default;
    ~DDSPdoPublisher() override = default;

    /**
     * @brief Initialize the DDS writer for a topic.
     * @return True if DDS initialization succeeds.
     */
    bool init(const std::string& topic_name,
              dds::domain::DomainParticipant& participant)
    {
        topic_name_ = topic_name;
        return this->init_dds(topic_name_, participant);
    }

    /// Return the DDS topic name configured during @ref init.
    const std::string& topic_name() const
    {
        return topic_name_;
    }

    /// Start assembling a fresh DDS message for a new adapter cycle.
    void begin_cycle() override{
        message_ = Msg{};
        has_update_ = false;
    }

    /// Convert and merge one received PDO into the current message.
    void consume(const Pdo& pdo) override
    {
        if(process(pdo)) {
            has_update_ = true;
        }
    }

    /**
     * @brief Publish the assembled message if the cycle is valid.
     *
     * No publication occurs when @p valid is false or no PDO updated the
     * message during the cycle.
     */
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

    /// Register the DDS writer with the ROS graph bridge.
    void connect_ros_graph_bridge(CycloneDDSRosGraphBridge &bridge) override {
        bridge.add_writer(this->writer_);
    }

protected:
    /**
     * @brief Convert a PDO and update the message being assembled.
     * @return True if @p pdo contributed to the message; otherwise false.
     */
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

/// Publishes converted IMU PDO data.
class ImuPublisher : public DDSPdoPublisher<ImuMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

/// Publishes the aggregated state of configured joints.
class JointStatePublisher : public DDSPdoPublisher<JointStateMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

/// Publishes converted motor PDO data.
class MotorsPublisher : public DDSPdoPublisher<MotorMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

/// Publishes converted power board PDO data.
class PowerBoardPublisher : public DDSPdoPublisher<PowerBoardMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

/// Publishes converted pump PDO data.
class PumpPublisher : public DDSPdoPublisher<PumpMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

/// Publishes converted force torque PDO data.
class ForceTorquePublisher : public DDSPdoPublisher<ForceTorqueMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

/// Publishes converted valve PDO data.
class ValvePublisher : public DDSPdoPublisher<ValveMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

/// Publishes converted gripper PDO data.
class GripperPublisher : public DDSPdoPublisher<GripperMsg> {
protected:
    bool process(const iit::advrf::Ec_slave_pdo& pdo) override;
};  

}