#pragma once

#include <ecat_master_pb/protobuf/ecat_pdo.pb.h>
#include <protobuf/repl_cmd.pb.h>
#include <shm_types.hpp>
#include <shm_utils.hpp>

#include "advrf_middleware_core/adapters/adapter_base.hpp"
#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/utils/log.hpp"

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
  template <typename Proto> bool push(advrf::middleware::shm::ChannelTx channel,Proto msg) {
    if (!is_ok() || !msg.has_header() || !msg.header().has_index() ||
        msg.header().index() < 0)
      return false;
    
    DeviceInfo dev_info{};
    dev_info.id = static_cast<uint32_t>(msg.header().index());

    return client_transport().push_tx_queue(dev_info, msg) > 0;
  }


  bool push(const uint32_t& id,const void* data,std::size_t size){
    const std::string string_id = std::to_string(id);
    DeviceInfo dev_info = {id,string_id};

    return client_transport().push_tx_queue(dev_info,data,size) > 0;
  }

};

}