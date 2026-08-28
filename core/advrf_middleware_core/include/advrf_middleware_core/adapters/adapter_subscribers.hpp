#pragma once

#include <shm_types.hpp>
#include <shm_utils.hpp>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/utils/log.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

namespace middleware_adapter::message {

class AdapterSubscribers : public AdapterBase {
public:
  AdapterSubscribers() = default;
  virtual ~AdapterSubscribers() = default;

  bool start() override {
    return shm_.connect(SHM_TX_PDO, ShmAttachMode::Open);
  }

  bool is_ok() const override { return shm_.is_ok(); }
  void close() override { shm_.close(); }

protected:
  template <typename Proto> bool push(ChannelTx channel, const Proto &msg) {
    auto device = device_for(channel);
    if (!device) {
      LOG_ERROR("No SHM device mapped for ChannelTx {}", static_cast<int>(channel));
      return false;
    }
    return shm_.push(*device, msg);
  }

private:
  ShmTxWriter shm_;
};

} // namespace middleware_adapter::message