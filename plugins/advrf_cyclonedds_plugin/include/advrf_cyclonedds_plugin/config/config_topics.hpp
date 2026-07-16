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

    std::string jointState()  const { return rt("jointState"); }
    std::string imu()         const { return rt("imu"); }
    std::string forceTorque() const { return rt("forceTorque"); }
    std::string motor()       const { return rt("motor"); }
    std::string powerBoard()  const { return rt("powerBoard"); }
    std::string pump()        const { return rt("pump"); }
    std::string valve()       const { return rt("valve"); }
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

struct TopicsServices : public TopicNamespace
{
    using TopicNamespace::TopicNamespace;

    std::string getCmdRequest()  const { return rq("parameters/getCmdRequest"); }
    std::string getCmdReply()    const { return rr("parameters/getCmdReply"); }
};

} // namespace topics

struct ConfigTopics
{
    explicit ConfigTopics(std::vector<std::string> ns)
        : state(ns),
          parameters(ns),
        srv(ns)
    {}

    topics::TopicsState state;
    topics::TopicsParameters parameters;
    topics::TopicsServices srv;
};

} // namespace config