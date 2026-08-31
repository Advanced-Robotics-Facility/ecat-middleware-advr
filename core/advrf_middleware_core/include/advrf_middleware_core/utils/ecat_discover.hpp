#pragma once
#include <shm_types.hpp>
#include <shm_utils.hpp>

#include "advrf_middleware_core/utils/channel.hpp"
#include "advrf_middleware_core/utils/pdo_utils.hpp"

#include <advrf_interfaces_protobuf/ecat_pdo.pb.h>

/**
 * @brief Metadata describing an EtherCAT device discovered from a received PDO.
 */
struct EcatMetadata {
  pdo_utils::EcatId ecat_id;              ///< Numeric EtherCAT device ID.
  std::string name;                       ///< Device identifier from the PDO header.
  iit::advrf::Ec_slave_pdo::Type type;    ///< Protobuf PDO type.
  ChannelRx channel;                      ///< Middleware receive channel.
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
  using EcatMap = std::unordered_map<pdo_utils::EcatId, EcatMetadata>;

  EcatDiscover() = default;
  ~EcatDiscover() = default;

  /**
   * @brief Connect to the shared-memory receive channel used for discovery.
   * @param shm_name Shared-memory channel name.
   * @return True if the connection succeeds.
   */
  bool start(const std::string &shm_name) {
    return shm_.connect(shm_name, ShmAttachMode::Open);
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

    while (ecat_map.size() < ecat_ids.size()) {
      discover_once(ecat_ids, ecat_map);
      LOG_INFO("Pdo discovery in progress, found {}/{} PDOs", ecat_map.size(), ecat_ids.size());
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return ecat_map;
  }

  /// Return whether the shared-memory connection is usable.
  bool is_ok() const { return shm_.is_ok(); }

  /// Close the shared-memory connection.
  void close() { shm_.close(); }

private:

  void discover_once(const std::set<uint32_t> &ecat_ids, EcatMap &ecat_map) {
    for (const auto channel : CHANNELS_ARRAY) {
      const auto device = device_for(channel);

      if (!device) {
        LOG_ERROR("Failed to get device for channel {}",
                  static_cast<int>(channel));
        continue;
      }

      std::vector<Pdo> pdos;
      shm_.peek_all(*device, pdos);

      for (const auto &pdo : pdos) {
        const auto &str_id = pdo.header().str_id();
        const int parsed_id = pdo_utils::get_ecat_id(str_id);

        if (parsed_id < 0) {
          LOG_ERROR("Format error for PDO frame with ID {}", str_id);
          continue;
        }

        const auto id = static_cast<uint32_t>(parsed_id);

        if (ecat_ids.find(id) == ecat_ids.end() ||
            ecat_map.find(id) != ecat_map.end()) {
          continue;
        }

        ecat_map.emplace(id, EcatMetadata{
                                .ecat_id = id,
                                .name = str_id,
                                .type = pdo.type(),
                                .channel = channel,
                                .device = *device,
                            });
      }
    }
  }

  ShmRxReader shm_;
};

/**
 * @brief Return the discovered PDO type for an EtherCAT ID.
 *
 * @return The PDO type, or @c Ec_slave_pdo_Type__UNSPECIFIED if the ID was not
 *         found.
 */
inline iit::advrf::Ec_slave_pdo::Type resolve_type(const EcatDiscover::EcatMap& ecat_map, 
                                                  pdo_utils::EcatId ecat_id) {
    auto it = ecat_map.find(ecat_id);
    if (it != ecat_map.end()) {
      return it->second.type;
    } else {
      LOG_ERROR("ECAT ID {} not found in PDO map", ecat_id);
      return iit::advrf::Ec_slave_pdo::Type::Ec_slave_pdo_Type__UNSPECIFIED;
    }
  }