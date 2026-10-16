#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include <advrf_middleware_core/adapters/adapter_base.hpp>
#include <advrf_middleware_core/config/config_file.hpp>

namespace advrf::middleware::adapters_mode
{

enum class MotorCommand : uint8_t
{
    Shutdown = 0,
    Start = 1,
    Stop = 2,
    SetGains = 3,
};

struct MotorControlRequest
{
    int32_t motor_id = -1;
    MotorCommand command = MotorCommand::Shutdown;
    uint32_t control_mode = 0;
    std::array<float, 5> gains{};
};

struct MotorControlReply
{
    bool success = false;
    // A timeout is ambiguous: the master may have executed the command.
    bool outcome_unknown = false;
    std::string message;
};

struct MotorControlConfig
{
    bool enabled = false;
    std::string command_uri = "tcp://127.0.0.1:5555";
    uint32_t timeout_ms = 1000;
    std::set<int32_t> allowed_motor_ids;
    std::set<int32_t> advrf_motor_ids; // Populated from device discovery.
};

MotorControlConfig parse_motor_control_config(const config::ConfigFile& file);
void validate(const MotorControlConfig& config);

/** Motor mode/activation commands over the master's existing ZeroMQ protocol.
 * No PDO transport or motor state machine is duplicated here. A successful
 * reply means master ACK, not confirmation of physical drive state.
 */
class MotorControlAdapter : public adapters::AdapterBase
{
public:
    explicit MotorControlAdapter(MotorControlConfig config);
    ~MotorControlAdapter() override;

    bool start() override;
    bool is_ok() const override;
    void close() override;

    void spin_once() override {}

    MotorControlReply execute(const MotorControlRequest& request);

private:
    struct Transport;
    MotorControlConfig config_;
    std::unique_ptr<Transport> transport_;
    mutable std::mutex mutex_;
};

} // namespace advrf::middleware::adapters_mode
