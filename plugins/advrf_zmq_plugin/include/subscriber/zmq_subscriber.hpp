#pragma once

#include <pb_utils.h>
#include <shm_utils.hpp>

#include <zmq.hpp>

#include <atomic>
#include <string>

namespace advrf::zmq_plugin
{

class ZmqSubscriber {
public:
    ZmqSubscriber(
        zmq::context_t& context,
        std::string endpoint)
        : _socket(context, zmq::socket_type::router)
    {
        _socket.set(
            zmq::sockopt::rcvhwm,
            64
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

        while (_running.load(std::memory_order_acquire)) {
            /*
            CommandEnvelope command;

            if (!receive_command(command)) {
                continue;
            }

            if (!transport.can_write_tx()) {
                if (!transport.try_acquire_tx()) {
                    send_reply(
                        command,
                        CommandStatus::TxUnavailable
                    );
                    continue;
                }
            }

            const bool pushed =
                transport.push_tx(
                    command.device_type,
                    command.slave_id,
                    command.payload
                );

            send_reply(
                command,
                pushed
                    ? CommandStatus::Accepted
                    : CommandStatus::QueueFull
            );
            */
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
