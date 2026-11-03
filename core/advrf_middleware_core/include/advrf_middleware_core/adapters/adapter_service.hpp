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
    ServiceTransport transport = ServiceTransport::Zmq;
    std::string command_uri = "tcp://127.0.0.1:5555";
    uint32_t timeout_ms = 1000;
};

/// Synchronous REPL service over ZeroMQ. SHM is unsupported by the v3 master.
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
        return execute_zmq(request);
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
        started_ = config_.transport == ServiceTransport::Zmq;
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
        return started_ && config_.transport == ServiceTransport::Zmq;
    }
    static iit::advrf::Cmd_reply make_nack(const iit::advrf::Repl_cmd& request,
                                          const std::string& message)
    {
        iit::advrf::Cmd_reply reply;
        reply.set_type(iit::advrf::Cmd_reply::NACK);
        reply.set_cmd_type(request.type());
        reply.set_msg(message);
        if (request.has_header())
            *reply.mutable_header() = request.header();
        return reply;
    }
    ServiceConfig config_;
    ServiceResult execute_zmq(const iit::advrf::Repl_cmd& request) const;
    mutable std::mutex request_mutex_;
    bool started_ = false;
};
} // namespace advrf::middleware::adapters::service
