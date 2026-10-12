#pragma once

#include <shm_types.hpp>
#include <shm_utils.hpp>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/utils/log.hpp"

#include <protobuf/ecat_pdo.pb.h>
#include <protobuf/repl_cmd.pb.h>

namespace advrf::middleware::adapters::message {

/**
 * @brief Base adapter for forwarding middleware messages to shared memory.
 *
 * Derived classes translate their message types and call @ref push to enqueue
 * them on the appropriate EtherCAT transmit channel.
 */
class AdapterSubscribers : public advrf::middleware::adapters::AdapterBase {
public:
  AdapterSubscribers() = default;
  virtual ~AdapterSubscribers() = default;

  /// Connect to the shared-memory transmit (TX) PDO channel.
  bool start() override {
    if (!client_transport().initialize_tx(Proto_type::PROTOBUFF)) {
      LOG_ERROR("Failed to open ready EtherCAT TX shared memory '{}'", SHM_TX_PDO);
      return false;
    }
    return true;
  }

  bool is_ok() const override { return client_transport().tx_is_ok(); }
  void close() override {}

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
  template <typename Proto> bool push(advrf::middleware::shm::ChannelTx channel, const Proto &msg) {
    auto device = advrf::middleware::shm::device_for(channel);
    if (!device) {
      LOG_ERROR("No SHM device mapped for ChannelTx {}", static_cast<int>(channel));
      return false;
    }

    return client_transport().push_tx_queue(*device, msg);
  }

};

}