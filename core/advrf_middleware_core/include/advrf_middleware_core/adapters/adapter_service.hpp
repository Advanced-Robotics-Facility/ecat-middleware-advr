#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <thread>

#include <shm_types.hpp>
#include <shm_utils.hpp>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/log.hpp"

#include <protobuf/ecat_pdo.pb.h>
#include <protobuf/repl_cmd.pb.h>
#include <advrf_middleware_core/utils/log.hpp>

namespace advrf::middleware::adapters::service
{

/**
 * @brief Synchronous service adapter for EtherCAT-master commands.
 *
 * For each request, the adapter sends a command through shared memory and
 * waits for the matching reply. Requests are serialized: only one can be
 * pending at a time.
 */
class AdapterServiceServer : public advrf::middleware::adapters::AdapterBase
{
public:
    AdapterServiceServer() = default;
    ~AdapterServiceServer() override = default;

    /**
     * @brief Send a command and wait for its reply.
     *
     * @param request Command to forward to the EtherCAT master.
     * @return The matching reply, or a NACK if shared memory is unavailable,
     *         full, or no reply arrives before @c ReplyTimeout.
     *
     * @note This function blocks for up to @c ReplyTimeout.
     */
    iit::advrf::Cmd_reply process_request(
        const iit::advrf::Repl_cmd& request)
    {
        // Only one outstanding SHM request at a time.
        std::scoped_lock lock{request_mutex_};

        if (!is_ok())
            return make_nack("ecat master not connected");

        if (!client_transport().push_request(request)) {
            LOG_ERROR("Push to request queue failed");
            return make_nack("shm request queue full");
        }

        LOG_DEBUG("Pushed request to SHM, waiting for reply...");
        const auto deadline = std::chrono::steady_clock::now() + ReplyTimeout;
        while (std::chrono::steady_clock::now() < deadline)
        {
            iit::advrf::Cmd_reply reply;
            if (!is_ok())
                return make_nack("ecat master disconnected");
            if (client_transport().try_pop_reply(reply) && matches(request, reply))
                return reply;
            std::this_thread::sleep_for(PollPeriod);
        }

        LOG_ERROR("Timeout waiting for reply from SHM");
        return make_nack(
            "Timeout waiting for reply from shm");
    }


    /// Connect to the shared-memory service channel.
    bool start() override
    {
        if (!client_transport().initialize_service()) {
            LOG_ERROR("Failed to open ready EtherCAT service shared memory '{}'", SHM_SRV_NAME);
            return false;
        }
        return true;
    }

    bool is_ok() const override { return client_transport().service_is_ok(); }
    void close() override {}

private:
    /// Maximum time to wait for a matching command reply.
    static constexpr auto ReplyTimeout = std::chrono::milliseconds{1000};

    /// Delay between consecutive reply-queue polls.
    static constexpr auto PollPeriod = std::chrono::microseconds{200};

    /// Check that a reply belongs to the given request.
    static bool matches(const iit::advrf::Repl_cmd& request,
                        const iit::advrf::Cmd_reply& reply)
    {
        return
            reply.request_id().guid() ==
                request.request_id().guid() &&
            reply.request_id().seq() ==
                request.request_id().seq();
    }

    /// Create a negative acknowledgement containing @p message.
    static iit::advrf::Cmd_reply make_nack(const std::string& message)
    {
        iit::advrf::Cmd_reply reply;

        reply.set_type(iit::advrf::Cmd_reply::NACK);
        reply.set_msg(message);

        return reply;
    }
    
    /// Serializes request/reply transactions.
    std::mutex request_mutex_;
};

} 