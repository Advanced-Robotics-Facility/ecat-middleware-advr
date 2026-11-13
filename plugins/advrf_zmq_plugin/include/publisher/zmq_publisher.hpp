#pragma once

#include <advrf_middleware_core/adapters/adapter_publishers.hpp>
#include <advrf_middleware_core/config/config_topics.hpp>
#include <advrf_middleware_core/config/robot_config.hpp>
#include <advrf_middleware_core/utils/ecat_discover.hpp>

#include <zmq.hpp>

#include <atomic>
#include <string>

namespace advrf::zmq_plugin
{
class ZmqPublisher : 
       public advrf::middleware::adapters::message::AdapterPublishers::IPublisher{
public:
    ZmqPublisher() = default;

    void initialize(zmq::context_t& context, const std::string& endpoint){

        _socket = std::make_unique<zmq::socket_t>(context, zmq::socket_type::pub);

        _socket->set(
            zmq::sockopt::sndhwm,
            static_cast<int>(SHARED_PUB_BRIDGE_SIZE_QUEUE));

        _socket->set(zmq::sockopt::linger, 0);
        _socket->bind(endpoint);
    }

private:

    void begin_cycle() {};
    void consume(const advrf::middleware::pdo::Pdo &pdo) {
        std::cout << pdo.ByteSizeLong()<< std::endl;
    };
    void end_cycle(bool valid) {};


    template<typename Slot>
    void publish(const Slot& slot)
    {
        const std::string topic = std::to_string(slot.slave_id);
        const std::size_t capacity = std::size(slot.buf);
        const std::size_t payload_size = static_cast<std::size_t>(slot.size);

        if (payload_size > capacity) {
            return;
        }

        zmq::message_t topic_frame(
            topic.data(),
            topic.size()
        );

        zmq::message_t payload_frame(
            std::data(slot.buf),
            payload_size
        );

        const auto topic_result = _socket->send(
            topic_frame,
            zmq::send_flags::sndmore |
                zmq::send_flags::dontwait
        );

        if (!topic_result) {
            ++_dropped_messages;
            return;
        }

        const auto payload_result = _socket->send(
            payload_frame,
            zmq::send_flags::dontwait
        );

        if (!payload_result) {
            ++_dropped_messages;
        }
    }

    std::unique_ptr<zmq::socket_t> _socket;
    std::uint64_t _dropped_messages{0};
};

class ZmqPublisherManager : 
       public advrf::middleware::adapters::message::AdapterPublishers::AdapterPublishers {
public:
    void setup(zmq::context_t& context,const std::string& endpoint)
    {
        
        using namespace advrf::middleware::shm;
        std::vector<ChannelRx> channels;
        channels.reserve(static_cast<std::size_t>(ChannelRx::Count));

        for (std::size_t i = 0; i < static_cast<std::size_t>(ChannelRx::Count); ++i) {
            channels.push_back(static_cast<ChannelRx>(i));
        }
        
        auto& publisher = register_publisher<ZmqPublisher>(std::move(channels));
        publisher.initialize(context,endpoint);
    }
};

}
