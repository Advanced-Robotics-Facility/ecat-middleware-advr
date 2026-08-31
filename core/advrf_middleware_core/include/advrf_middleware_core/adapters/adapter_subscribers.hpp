#pragma once

#include <shm_types.hpp>
#include <shm_utils.hpp>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/utils/log.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>
#include <advrf_interfaces_protobuf/repl_cmd.pb.h>

namespace middleware_adapter::message {

/**
 * @brief Base adapter for forwarding middleware messages to shared memory.
 *
 * Derived classes translate their message types and call @ref push to enqueue
 * them on the appropriate EtherCAT transmit channel.
 */
class AdapterSubscribers : public AdapterBase {
public:
  AdapterSubscribers() = default;
  virtual ~AdapterSubscribers() = default;

  /// Connect to the shared-memory transmit (TX) PDO channel.
  bool start() override {
    return shm_.connect(SHM_TX_PDO, ShmAttachMode::Open);
  }

  bool is_ok() const override { return shm_.is_ok(); }
  void close() override { shm_.close(); }

protected:
  /**
   * @brief Enqueue a Protobuf message on a transmit channel.
   *
   * @tparam Proto Protobuf message type accepted by the selected channel.
   * @param channel Destination transmit channel.
   * @param msg Message to enqueue.
   * @return True if a matching shared-memory device exists and accepts the
   *         message; otherwise false.
   */
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

}