#include <advrf_middleware_core/adapters_mode/motor_control.hpp>

#include <cmath>
#include <limits>
#include <protobuf/repl_cmd.pb.h>
#include <stdexcept>
#include <utility>
#include <zmq.hpp>
#include <zmq_addon.hpp>

namespace advrf::middleware::adapters_mode
{

MotorControlConfig parse_motor_control_config(const config::ConfigFile& file)
{
    const auto modes = config::section(file.root, "adapters_mode");
    config::check_keys(modes, {"motor_control"}, "adapters_mode");
    const auto node = config::section(file.root, "motor_control");
    config::check_keys(node, {"command_uri", "timeout_ms", "allowed_motor_ids"}, "motor_control");

    MotorControlConfig result;
    result.enabled = config::value<bool>(modes, "motor_control", false, "adapters_mode");
    result.command_uri =
        config::value<std::string>(node, "command_uri", result.command_uri, "motor_control");
    result.timeout_ms =
        config::unsigned_value(node, "timeout_ms", result.timeout_ms, "motor_control");

    if (const auto ids = node["allowed_motor_ids"])
    {
        if (!ids.IsSequence())
        {
            throw config::ConfigError("motor_control.allowed_motor_ids: expected a sequence");
        }

        for (const auto& id : ids)
        {
            const auto number = id.as<int32_t>();
            if (number < 0 || !result.allowed_motor_ids.insert(number).second)
            {
                throw config::ConfigError(
                    "motor_control.allowed_motor_ids: expected unique nonnegative IDs");
            }
        }
    }

    validate(result);
    return result;
}

void validate(const MotorControlConfig& config)
{
    if (config.command_uri.empty() || config.timeout_ms == 0 || config.timeout_ms > 60000)
    {
        throw config::ConfigError(
            "motor_control: provide command_uri and timeout_ms between 1 and 60000");
    }

    for (const auto id : config.allowed_motor_ids)
    {
        if (id < 0)
        {
            throw config::ConfigError("motor_control.allowed_motor_ids: negative ID");
        }
    }
}

struct MotorControlAdapter::Transport
{
    zmq::context_t context{1};
};

MotorControlAdapter::MotorControlAdapter(MotorControlConfig config) : config_(std::move(config))
{
    validate(config_);
}

MotorControlAdapter::~MotorControlAdapter() = default;

bool MotorControlAdapter::start()
{
    std::scoped_lock lock(mutex_);
    if (!config_.enabled)
    {
        return false;
    }

    transport_ = std::make_unique<Transport>();
    return true;
}

bool MotorControlAdapter::is_ok() const
{
    std::scoped_lock lock(mutex_);
    return transport_ != nullptr;
}

void MotorControlAdapter::close()
{
    std::scoped_lock lock(mutex_);
    transport_.reset();
}

MotorControlReply MotorControlAdapter::execute(const MotorControlRequest& request)
{
    std::scoped_lock lock(mutex_);

    if (!transport_)
    {
        return {false, false, "Motor control adapter is not started"};
    }

    if (request.motor_id < 0 || (!config_.allowed_motor_ids.empty() &&
                                    config_.allowed_motor_ids.count(request.motor_id) == 0))
    {
        return {false, false, "Motor ID is not allowed"};
    }

    iit::advrf::Repl_cmd envelope;
    envelope.set_type(iit::advrf::CTRL_CMD);
    auto* command = envelope.mutable_ctrl_cmd();
    command->set_board_id(request.motor_id);

    switch (request.command)
    {
    case MotorCommand::Start:
        command->set_type(iit::advrf::Ctrl_cmd::CTRL_CMD_START);
        command->set_value(request.control_mode);
        break;
    case MotorCommand::Stop:
    case MotorCommand::Shutdown:
        command->set_type(iit::advrf::Ctrl_cmd::CTRL_CMD_STOP);
        break;
    case MotorCommand::SetGains:
        command->set_type(iit::advrf::Ctrl_cmd::CTRL_SET_GAINS);
        break;
    default:
        return {false, false, "Unknown motor command"};
    }

    if (request.command == MotorCommand::Start || request.command == MotorCommand::SetGains)
    {
        if (request.control_mode > static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
            request.control_mode == 0 || !iit::advrf::Gains_Type_IsValid(request.control_mode))
        {
            return {false, false, "Invalid control mode"};
        }

        for (const auto gain : request.gains)
        {
            if (!std::isfinite(gain))
            {
                return {false, false, "Gains must be finite"};
            }
        }

        auto ordered_gains = request.gains;
        if (config_.advrf_motor_ids.count(request.motor_id) != 0 &&
            (request.control_mode == iit::advrf::Gains_Type_POSITION ||
                request.control_mode == iit::advrf::Gains_Type_VELOCITY))
        {
            // Match EcReplCmd::check_advrf_motor_gains for ADVRF drives.
            ordered_gains = {request.gains[0], request.gains[2], 0.0f, 0.0f, request.gains[1]};
        }

        auto* gains = command->mutable_gains();
        gains->set_type(static_cast<iit::advrf::Gains_Type>(request.control_mode));
        gains->set_pos_kp(ordered_gains[0]);
        gains->set_pos_kd(ordered_gains[1]);
        gains->set_tor_kp(ordered_gains[2]);
        gains->set_tor_kd(ordered_gains[3]);
        gains->set_tor_ki(ordered_gains[4]);
    }

    bool sent = false;

    try
    {
        // A fresh REQ socket per transaction prevents a late reply after a
        // timeout from being mistaken for the next command's acknowledgement.
        zmq::socket_t socket(transport_->context, ZMQ_REQ);
        socket.set(zmq::sockopt::linger, 0);
        socket.set(zmq::sockopt::sndtimeo, static_cast<int>(config_.timeout_ms));
        socket.set(zmq::sockopt::rcvtimeo, static_cast<int>(config_.timeout_ms));
        socket.connect(config_.command_uri);

        zmq::multipart_t outgoing;
        // The master pops the routing header first, then the Protobuf body.
        // EcReplCmd uses push_front twice; addstr appends, so order is explicit.
        outgoing.addstr("ESC_CMD");
        outgoing.addstr(envelope.SerializeAsString());

        if (!outgoing.send(socket))
        {
            return {false, false, "ZeroMQ send failed"};
        }

        sent = true;
        zmq::message_t incoming;
        if (!socket.recv(incoming))
        {
            return {
                false, true, "Master reply timeout; command outcome unknown; no automatic retry"};
        }

        iit::advrf::Cmd_reply reply;
        if (!reply.ParseFromArray(incoming.data(), static_cast<int>(incoming.size())) ||
            reply.cmd_type() != iit::advrf::CTRL_CMD ||
            (reply.type() != iit::advrf::Cmd_reply::ACK &&
                reply.type() != iit::advrf::Cmd_reply::NACK))
        {
            return {false, true, "Malformed or unexpected master reply; command outcome unknown"};
        }

        return {reply.type() == iit::advrf::Cmd_reply::ACK, false, reply.msg()};
    }
    catch (const zmq::error_t& error)
    {
        return {false, sent, std::string("ZeroMQ: ") + error.what()};
    }
}

} // namespace advrf::middleware::adapters_mode
