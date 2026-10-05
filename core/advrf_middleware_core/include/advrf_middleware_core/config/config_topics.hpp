#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace advrf::middleware::config
{

/**
 * @brief Base class for building middleware topic names.
 *
 * Topic names have the form:
 * @code
 * <prefix>/<namespace...>/<suffix>
 * @endcode
 *
 * The prefixes @c rt, @c rq, and @c rr identify ROS topic (rt/), ROS service 
 * request (rq/), and ROS service reply (rr/) topics respectively.
 * 
 * @note There exists also ra/ and rp/ which identify ROS actions
 * and ROS parameters.
 */
struct TopicNamespace
{
    /**
     * @brief Create a topic builder with the given namespace components.
     *
     * For example, @c {"advrf", "kyon"} becomes @c advrf/kyon in every topic.
     */
    explicit TopicNamespace(std::vector<std::string> ns)
        : namespace_(std::move(ns))
    {}

protected:
    /// Build a ROS topic with the given suffix.
    std::string rt(std::string_view suffix) const
    {
        return build("rt", suffix);
    }

    /// Build a ROS service request topic with the given suffix.
    std::string rq(std::string_view suffix) const
    {
        return build("rq", suffix);
    }

    /// Build a ROS service reply topic with the given suffix.
    std::string rr(std::string_view suffix) const
    {
        return build("rr", suffix);
    }

private:
    std::string build(std::string_view prefix,
                      std::string_view suffix) const
    {
        std::string topic(prefix);

        for (const auto& ns : namespace_)
        {
            topic += '/';
            topic += ns;
        }

        topic += '/';
        topic += suffix;

        return topic;
    }

    std::vector<std::string> namespace_;
};

namespace topics
{

/// ROS state topics published by the EtherCAT middleware.
struct TopicsState : public TopicNamespace
{
    using TopicNamespace::TopicNamespace;

    std::string jointState()  const { return rt("rx/joint_states"); }
    std::string motor()       const { return rt("rx/motors"); }
    std::string gripper()     const { return rt("rx/grippers"); }
    std::string valve()       const { return rt("rx/valves"); }

    std::string imu(const std::string& device_name)         const { return rt("rx/imu/" + device_name); }
    std::string forceTorque(const std::string& device_name) const { return rt("rx/force_torque/" + device_name); }
    std::string powerBoard(const std::string& device_name)  const { return rt("rx/power_board/" + device_name); }
    std::string pump(const std::string& device_name)        const { return rt("rx/pump/" + device_name); }
};

/// ROS command topics received by the EtherCAT middleware.
struct TopicsCommand  : public TopicNamespace
{
    using TopicNamespace::TopicNamespace;
    std::string jointCmd()   const { return rt("tx/joints"); }
    std::string motorCmd()   const { return rt("tx/motors"); }
    std::string valveCmd()   const { return rt("tx/valves"); }
    std::string gripperCmd() const { return rt("tx/grippers"); }
    std::string pumpCmd()    const { return rt("tx/pumps"); }
    std::string powerBoardCmd()    const { return rt("tx/power_board"); }
    std::string forceTorqueCmd()    const { return rt("tx/force_torque"); }
};

/// Request and reply topics for EtherCAT service commands.
struct TopicsService : public TopicNamespace
{
    using TopicNamespace::TopicNamespace;

    std::string request() const { return rq("replCmd/request"); }
    std::string reply()   const { return rr("replCmd/reply"); }
};

/// Request and reply topics used to access middleware parameters.
struct TopicsParameters : public TopicNamespace
{
    using TopicNamespace::TopicNamespace;

    std::string getRequest()  const { return rq("parameters/getRequest"); }
    std::string getReply()    const { return rr("parameters/getReply"); }

    std::string getCmdRequest()  const { return rq("parameters/getCmdRequest"); }
    std::string getCmdReply()    const { return rr("parameters/getCmdReply"); }

    std::string setRequest()  const { return rq("parameters/setRequest"); }
    std::string setReply()    const { return rr("parameters/setReply"); }

    std::string listRequest() const { return rq("parameters/listRequest"); }
    std::string listReply()   const { return rr("parameters/listReply"); }

    std::string listGetRequest() const { return rq("parameters/listGetRequest"); }
    std::string listGetReply()   const { return rr("parameters/listGetReply"); }
};

} // namespace topics

/**
 * @brief Complete collection of middleware topic-name builders.
 *
 * All groups share the same namespace prefix.
 */
struct ConfigTopics
{
    explicit ConfigTopics(std::vector<std::string> ns)
        : rx(ns),
          tx(ns),
          parameters(ns),
          service(ns)
    {}

    topics::TopicsState rx;
    topics::TopicsCommand tx;
    topics::TopicsParameters parameters;
    topics::TopicsService service;
};

} // namespace config
