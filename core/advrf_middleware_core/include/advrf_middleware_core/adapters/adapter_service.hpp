#pragma once

#include <unistd.h>
#include <chrono>
#include <thread>
#include <shm_utils.hpp>
#include <mutex>
#include <string>
#include <utility>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include <protobuf/repl_cmd.pb.h>

namespace advrf::middleware::adapters::service
{

struct ServiceResult
{
    bool success = false;
    bool outcome_unknown = false;
    std::string message;
    iit::advrf::Cmd_reply reply;
};

enum class ServiceTransport { Shm, Zmq };

struct ServiceConfig
{
    ServiceTransport transport = ServiceTransport::Shm;
    std::string command_uri = "tcp://127.0.0.1:5555";
    uint32_t timeout_ms = 1000;
};

/// Synchronous REPL service with an explicitly selected SHM or ZeroMQ transport.
/// A missing reply does not imply that the command was not executed.
class AdapterServiceServer : public advrf::middleware::adapters::AdapterBase
{
public:
    explicit AdapterServiceServer(ServiceConfig config = {});
    ~AdapterServiceServer() override = default;

    ServiceResult process_request_result(const iit::advrf::Repl_cmd& request)
    {
        std::scoped_lock lock{request_mutex_};
        if (!ready())
            return {false, false, "REPL service is not started"};
        if (config_.transport == ServiceTransport::Zmq)
            return execute_zmq(request);

        // All adapters share the same SHM request/reply queues.
        std::scoped_lock shm_lock{shm_request_mutex_};
        auto forwarded = request;
        forwarded.mutable_request_id()->set_guid(static_cast<uint32_t>(::getpid()));
        forwarded.mutable_request_id()->set_seq(++shm_sequence_);
        if (!client_transport().push_request(forwarded))
            return {false, false, "SHM request queue full"};
        const auto deadline = std::chrono::steady_clock::now() +
                              std::chrono::milliseconds(config_.timeout_ms);
        while (std::chrono::steady_clock::now() < deadline)
        {
            if (!ready())
                return {false, true, "SHM disconnected; command outcome unknown; no automatic retry"};
            iit::advrf::Cmd_reply reply;
            if (client_transport().try_pop_reply(reply) &&
                reply.request_id().guid() == forwarded.request_id().guid() &&
                reply.request_id().seq() == forwarded.request_id().seq() &&
                reply.cmd_type() == request.type() &&
                (reply.type() == iit::advrf::Cmd_reply::ACK ||
                 reply.type() == iit::advrf::Cmd_reply::NACK))
            {
                *reply.mutable_request_id() = request.request_id();
                return {reply.type() == iit::advrf::Cmd_reply::ACK, false, reply.msg(), reply};
            }
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
        return {false, true, "SHM reply timeout; command outcome unknown; no automatic retry"};
    }

    iit::advrf::Cmd_reply process_request(const iit::advrf::Repl_cmd& request)
    {
        const auto result = process_request_result(request);
        if (result.reply.type() == iit::advrf::Cmd_reply::ACK ||
            result.reply.type() == iit::advrf::Cmd_reply::NACK)
            return result.reply;
        return make_nack(request, result.message);
    }

    void spin_once() override {}

    /// Local readiness only; does not send a command to the master.
    bool start() override
    {
        std::scoped_lock lock{request_mutex_};
        started_ = config_.transport == ServiceTransport::Zmq ||
                   client_transport().initialize_service();
        return started_;
    }
    bool is_ok() const override
    {
        std::scoped_lock lock{request_mutex_};
        return ready();
    }
    void close() override
    {
        std::scoped_lock lock{request_mutex_};
        started_ = false;
    }

private:
    bool ready() const
    {
        return started_ && (config_.transport == ServiceTransport::Zmq ||
                            client_transport().service_is_ok());
    }
    static iit::advrf::Cmd_reply make_nack(const iit::advrf::Repl_cmd& request,
                                          const std::string& message)
    {
        iit::advrf::Cmd_reply reply;
        reply.set_type(iit::advrf::Cmd_reply::NACK);
        reply.set_cmd_type(request.type());
        reply.set_msg(message);
        *reply.mutable_request_id() = request.request_id();
        return reply;
    }
    ServiceConfig config_;
    ServiceResult execute_zmq(const iit::advrf::Repl_cmd& request) const;
    inline static std::mutex shm_request_mutex_;
    inline static uint32_t shm_sequence_ = 0;
    mutable std::mutex request_mutex_;
    bool started_ = false;
};
} // namespace advrf::middleware::adapters::service
