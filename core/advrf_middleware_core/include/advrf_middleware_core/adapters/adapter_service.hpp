#pragma once

#include <shm_types.hpp>
#include <shm_utils.hpp>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/log.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>
#include <advrf_middleware_core/utils/log.hpp>

namespace middleware_adapter::service
{


class AdapterServiceServer : public AdapterBase
{
public:
    AdapterServiceServer() = default;
    ~AdapterServiceServer() override = default;


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


    bool start() override
    {
        return shm_.connect_and_wait(
            SHM_REPL_NAME,
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
    static constexpr auto ReplyTimeout =
        std::chrono::milliseconds{1000};

    static constexpr auto PollPeriod =
        std::chrono::microseconds{200};


    static bool matches(
        const iit::advrf::Repl_cmd& request,
        const iit::advrf::Cmd_reply& reply)
    {
        return
            reply.request_id().guid() ==
                request.request_id().guid() &&
            reply.request_id().seq() ==
                request.request_id().seq();
    }


    static iit::advrf::Cmd_reply make_nack(
        const std::string& message)
    {
        iit::advrf::Cmd_reply reply;

        reply.set_type(iit::advrf::Cmd_reply::NACK);
        reply.set_msg(message);

        return reply;
    }
    
    std::mutex request_mutex_;
};


} // namespace middleware_adapter::service