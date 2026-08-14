#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace config
{

struct TopicNamespace
{
    explicit TopicNamespace(std::vector<std::string> ns)
        : namespace_(std::move(ns))
    {}

protected:
    std::string rt(std::string_view suffix) const
    {
        return build("rt", suffix);
    }

    std::string rq(std::string_view suffix) const
    {
        return build("rq", suffix);
    }

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


struct TopicsCommand  : public TopicNamespace
{
    using TopicNamespace::TopicNamespace;
    std::string jointCmd()  const { return rt("tx/joints"); }
    std::string motorXtCmd()  const { return rt("tx/motors_xt"); }
    std::string motorCmd()  const { return rt("tx/motors"); }
};


struct TopicsService : public TopicNamespace
{
    using TopicNamespace::TopicNamespace;

    std::string request() const { return rq("replCmd/request"); }
    std::string reply()   const { return rr("replCmd/reply"); }
};

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
