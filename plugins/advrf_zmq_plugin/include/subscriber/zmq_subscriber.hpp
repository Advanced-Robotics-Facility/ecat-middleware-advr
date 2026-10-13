#pragma once

#include <pb_utils.h>
#include <shm_utils.hpp>

#include <zmq.hpp>

#include <atomic>
#include <string>
#include <charconv>

namespace advrf::zmq_plugin
{

class ZmqSubscriber {
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

    void run()
    {
        auto& transport = client_transport();

        if (!_socket) {
            return;
        }

        while (_running.load(std::memory_order_acquire)) {
            
            zmq::message_t identity_frame;
            zmq::message_t slave_id_frame;
            zmq::message_t payload_frame;

            try {
                // Blocks without busy-spinning.
                const auto identity_result = _socket.recv(identity_frame,zmq::recv_flags::none);

                // Timeout, if rcvtimeo is configured.
                if (!identity_result) {
                    continue;
                }

                if (!identity_frame.more()) {
                    continue;
                }

                if (!_socket.recv(slave_id_frame,zmq::recv_flags::none)) {
                    continue;
                }

                if (!slave_id_frame.more()) {
                    continue;
                }

                if (!_socket.recv(payload_frame,zmq::recv_flags::none)) {
                    continue;
                }

                int32_t slave_id{0};

                const auto* first =static_cast<const char*>(slave_id_frame.data());
                const auto* last = first + slave_id_frame.size();

                const auto result = std::from_chars(first, last, slave_id);

                if (result.ec != std::errc{} || result.ptr != last || slave_id == 0) {
                    continue;
                }

                const std::string string_id = std::to_string(slave_id);

                DeviceInfo dev_info = {slave_id,string_id,DeviceType::MOTOR};

                const int ret = transport.push_tx_queue(
                    dev_info,
                    payload_frame.data(),
                    payload_frame.size()
                );

            }
            catch (const zmq::error_t& error) {
                if (!_running.load(std::memory_order_acquire)) {
                    break;
                }

                std::cerr << "Router receive error: "<< error.what() << '\n';
            }
        }
    }

    void stop()
    {
        _running.store(false,std::memory_order_release);
    }

private:
    zmq::socket_t _socket;
    std::atomic<bool> _running{true};
};

}
