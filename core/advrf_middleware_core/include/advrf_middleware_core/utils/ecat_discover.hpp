#pragma once
#include <shm_types.hpp>
#include <shm_utils.hpp>

#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/utils/pdo_utils.hpp"

#include <ecat_master_pb/protobuf/ecat_pdo.pb.h>

namespace advrf::middleware::ecat {

/**
 * @brief Metadata describing an EtherCAT device discovered from a received PDO.
 */
struct EcatMetadata {
  advrf::middleware::pdo::EcatId ecat_id;              ///< Numeric EtherCAT device ID.
  std::string name;                       ///< Device identifier from the PDO header.
  iit::advrf::Ec_slave_pdo::Type type;    ///< Protobuf PDO type.
  advrf::middleware::shm::ChannelRx channel;                      ///< Middleware receive channel.
  DeviceTypeRx device;                    ///< Corresponding shared-memory device.
};

/**
 * @brief Discovers EtherCAT devices by inspecting PDOs in shared memory.
 *
 * The caller supplies the expected EtherCAT IDs. Discovery completes only
 * once a PDO has been observed for every requested ID.
 */
class EcatDiscover {
public:
  using Pdo = iit::advrf::Ec_slave_pdo;
  using EcatMap = std::unordered_map<advrf::middleware::pdo::EcatId, EcatMetadata>;

  EcatDiscover() = default;
  ~EcatDiscover() = default;

  /**
   * @brief Connect to the shared-memory receive channel used for discovery.
   * @param shm_name Shared-memory channel name.
   * @return True if the connection succeeds.
   */
  bool start(const std::string &shm_name) {
    if (shm_name != SHM_NRT_RX_PDO) {
      LOG_ERROR("Unsupported EtherCAT RX shared memory '{}'", shm_name);
      return false;
    }

    if (!client_transport().initialize_rx(false, Proto_type::PROTOBUFF)) {
      LOG_ERROR("Failed to open ready EtherCAT RX shared memory '{}'", shm_name);
      return false;
    }

    return true;
  }

  /**
   * @brief Discover metadata for every requested EtherCAT ID.
   *
   * @param ecat_ids IDs expected in the received PDO stream.
   * @return Metadata indexed by EtherCAT ID.
   *
   * @note Blocks until all requested IDs have been observed.
   */
  EcatMap discover(const std::set<uint32_t> &ecat_ids) {
    EcatMap ecat_map;

    if (!is_ok()) {
      LOG_ERROR("Cannot discover EtherCAT devices without RX shared memory");
      return ecat_map;
    }

    while (ecat_map.size() < ecat_ids.size()) {
      discover_once(ecat_ids, ecat_map);
      LOG_INFO("Pdo discovery in progress, found {}/{} PDOs", ecat_map.size(), ecat_ids.size());
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return ecat_map;
  }

  /// Return whether the shared-memory connection is usable.
  bool is_ok() const {
    return client_transport().rx_is_ok();
  }

  /// Close the shared-memory connection.
  void close() {}

private:

  void discover_once(const std::set<uint32_t> &ecat_ids, EcatMap &ecat_map) {
    for (const auto channel : advrf::middleware::shm::CHANNELS_ARRAY) {
      const auto device = advrf::middleware::shm::device_for(channel);

      if (!device) {
        LOG_ERROR("Failed to get device for channel {}",
                  static_cast<int>(channel));
        continue;
      }

      Pdo pdo;

      client_transport().peek_rx_queue(*device, pdo, [&](const Pdo& received) {
        if (!received.has_header() || !received.header().has_index()) {
          LOG_ERROR("Received PDO without EtherCAT index during discovery");
          return;
        }

        const auto parsed_id = received.header().index();
        if (parsed_id < 0)
          return;

        const auto id = static_cast<uint32_t>(parsed_id);
        if (ecat_ids.find(id) == ecat_ids.end() ||
            ecat_map.find(id) != ecat_map.end()) {
          return;
        }

        const std::string name =
            received.header().has_str_id() && !received.header().str_id().empty()
                ? received.header().str_id()
                : "ecat_" + std::to_string(id);

        ecat_map.emplace(id, EcatMetadata{
                                .ecat_id = id,
                                .name = name,
                                .type = received.type(),
                                .channel = channel,
                                .device = *device,
                            });
      });
    }
  }

};

/**
 * @brief Return the discovered PDO type for an EtherCAT ID.
 *
 * @return The PDO type, or @c Ec_slave_pdo_Type__UNSPECIFIED if the ID was not
 *         found.
 */
inline iit::advrf::Ec_slave_pdo::Type resolve_type(const EcatDiscover::EcatMap& ecat_map, 
                                                   advrf::middleware::pdo::EcatId ecat_id) {
    auto it = ecat_map.find(ecat_id);
    if (it != ecat_map.end()) {
      return it->second.type;
    } else {
      LOG_ERROR("ECAT ID {} not found in PDO map", ecat_id);
      return iit::advrf::Ec_slave_pdo::Type::Ec_slave_pdo_Type__UNSPECIFIED;
    }
  }

}
