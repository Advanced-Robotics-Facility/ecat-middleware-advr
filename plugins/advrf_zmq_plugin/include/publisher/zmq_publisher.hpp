#pragma once

#include <pb_utils.h>
#include <shm_utils.hpp>

#include <zmq.hpp>

#include <atomic>
#include <string>
#include <thread>

namespace advrf::zmq_plugin
{
class ZmqPublisher {
public:
    ZmqPublisher(
        zmq::context_t& context,
        std::string endpoint)
        : _socket(context, zmq::socket_type::pub)
    {
        _socket.set(
            zmq::sockopt::sndhwm,
            10
        );

        _socket.set(
            zmq::sockopt::linger,
            0
        );

        _socket.bind(endpoint);
    }

    void run()
    {
        auto& transport = client_transport();

        using Clock = std::chrono::steady_clock;

        constexpr auto period = std::chrono::milliseconds(2);
        auto next_wakeup = Clock::now();

        while (_running.load(std::memory_order_acquire)) {
            transport.drain_rx_queues(
                [&](const auto& slot) {
                    publish(slot);
                }
            );

            next_wakeup += period;
            std::this_thread::sleep_until(next_wakeup);

            const auto now = Clock::now();
    
            if (now > next_wakeup + period) {
                next_wakeup = now;
            }

        }
    }

    void stop()
    {
        _running.store(false,std::memory_order_release);
    }

private:
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

        const auto topic_result = _socket.send(
            topic_frame,
            zmq::send_flags::sndmore |
                zmq::send_flags::dontwait
        );

        if (!topic_result) {
            ++_dropped_messages;
            return;
        }

        const auto payload_result = _socket.send(
            payload_frame,
            zmq::send_flags::dontwait
        );

        if (!payload_result) {
            ++_dropped_messages;
        }
    }

    zmq::socket_t _socket;
    std::atomic<bool> _running{true};
    std::uint64_t _dropped_messages{0};
};
}
