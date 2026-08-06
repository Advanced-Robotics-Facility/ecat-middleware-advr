#pragma once

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/shared_memory/shm_connection_subscribers.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

namespace middleware_adapter::message {

class AdapterSubscribers : public AdapterBase
{
public:

    AdapterSubscribers(){}
    ~AdapterSubscribers() override = default;

    void forward(const iit::advrf::Repl_cmd_vector& cmd){
        for(auto it = cmd.requests().begin(); it != cmd.requests().end(); ++it){
             shm_.push_request(*it);
        }
    }

    void forward(const iit::advrf::Repl_cmd& cmd){
        shm_.push_request(cmd);
    }

    SubscriberShmConnection& shm() noexcept
    {
        return shm_;
    }

    const SubscriberShmConnection& shm() const noexcept
    {
        return shm_;
    }

    bool start() override
    {
        return shm_.connect(SHM_TX_PDO);
    }

    bool is_ok() const override
    {
        return shm_.is_ok();
    }

protected:
    SubscriberShmConnection shm_;

   
private:
   
};

} // namespace middleware_adapter::message