#pragma once


#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/shared_memory/shm_connection_subscribers.hpp"

#include <ecat_master_future/shm_shared_types.hpp>
#include <ecat_master_future/shm_utils.hpp>

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

namespace middleware_adapter::message {

class AdapterSubscribers : public AdapterBase
{
public:

    AdapterSubscribers(){}
    ~AdapterSubscribers() override = default;

    void forward_ctrl_cmd(const iit::advrf::Motors_PDO_cmd& cmd){
        LOG_INFO("Forwarding control command..., size: {}", cmd.motors_pdo().size());
        for(auto it = cmd.motors_pdo().begin(); it != cmd.motors_pdo().end(); ++it){
            LOG_INFO("Recevied message: {}", 
                it->motor_id());
        }
        shm_.push_motors_pdo(cmd);
    }

    SubscriberShmConnection& shm() noexcept
    {
        return shm_;
    }

    const SubscriberShmConnection& shm() const noexcept
    {
        return shm_;
    }

protected:
    SubscriberShmConnection shm_;

   
private:
   
};

} // namespace middleware_adapter::message