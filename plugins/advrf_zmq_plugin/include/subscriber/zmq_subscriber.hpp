#pragma once

#include <pb_utils.h>
#include <shm_utils.hpp>

#include <zmq.hpp>

#include <atomic>
#include <string>
#include <charconv>

#include <advrf_middleware_core/adapters/adapter_subscribers.hpp>

namespace advrf::zmq_plugin
{

class ZmqSubscriber: public advrf::middleware::adapters::message::AdapterSubscribers{
public:
    ZmqSubscriber(
        zmq::context_t& context,
        std::string endpoint)
        : _socket(context, zmq::socket_type::router)
    {
        _socket.set(zmq::sockopt::linger, 0);
        _socket.set(zmq::sockopt::rcvtimeo, 100);
        _socket.bind(endpoint);
    }

    void spin_once() override
    {
        if (!_socket) {
            return;
        }

        zmq::message_t identity_frame;
        zmq::message_t slave_id_frame;
        zmq::message_t payload_frame;

        try {
            // Blocks without busy-spinning.
            const auto identity_result = _socket.recv(identity_frame,zmq::recv_flags::none);

            // Timeout, if rcvtimeo is configured.
            if (!identity_result) {
                return;
            }

            if (!identity_frame.more()) {
                return;
            }

            if (!_socket.recv(slave_id_frame,zmq::recv_flags::none)) {
                return;
            }

            if (!slave_id_frame.more()) {
                return;
            }

            if (!_socket.recv(payload_frame,zmq::recv_flags::none)) {
                return;
            }

            int32_t slave_id{0};

            const auto* first =static_cast<const char*>(slave_id_frame.data());
            const auto* last = first + slave_id_frame.size();

            const auto result = std::from_chars(first, last, slave_id);

            if (result.ec != std::errc{} || result.ptr != last || slave_id == 0) {
                return;
            }

            const int ret = push(
                slave_id,
                payload_frame.data(),
                payload_frame.size()
            );

        }
        catch (const zmq::error_t& error) {
            std::cerr << "Router receive error: "<< error.what() << '\n';
        }

    }

private:
    zmq::socket_t _socket;
};

}
