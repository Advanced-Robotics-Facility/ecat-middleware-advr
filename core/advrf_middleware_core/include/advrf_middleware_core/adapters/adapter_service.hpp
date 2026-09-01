#pragma once

#include <shm_types.hpp>
#include <shm_utils.hpp>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/log.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
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

        if (!shm_.is_ok())
            return make_nack("ecat master not connected");

        if (!shm_.push_request(request))
        {
            LOG_ERROR("Push to request queue failed");
            return make_nack("shm request queue full");
        }

        LOG_DEBUG("Pushed request to SHM, waiting for reply...");
        const auto deadline = std::chrono::steady_clock::now() + ReplyTimeout;
        while (std::chrono::steady_clock::now() < deadline)
        {
            iit::advrf::Cmd_reply reply;
            if (shm_.try_pop_reply(reply))
            {
                if (matches(request, reply))
                    return reply;
            }
            std::this_thread::sleep_for(PollPeriod);
        }

        LOG_ERROR("Timeout waiting for reply from SHM");
        return make_nack(
            "Timeout waiting for reply from shm");
    }


    ShmServiceClient& shm() noexcept
    {
        return shm_;
    }


    const ShmServiceClient& shm() const noexcept
    {
        return shm_;
    }

    /// Connect to the shared-memory service channel.
    bool start() override
    {
        return shm_.connect_and_wait(
            SHM_SERVICE,
            ShmAttachMode::Open);
    }


    bool is_ok() const override
    {
        return shm_.is_ok();
    }

    void close() override
    {
        shm_.close();
    }


protected:
    ShmServiceClient shm_;


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