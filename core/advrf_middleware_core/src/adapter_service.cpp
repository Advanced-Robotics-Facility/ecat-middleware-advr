#include <advrf_middleware_core/adapters/adapter_service.hpp>
#include <stdexcept>
#include <utility>
#include <zmq.hpp>
#include <zmq_addon.hpp>

namespace advrf::middleware::adapters::service
{
AdapterServiceServer::AdapterServiceServer(ServiceConfig config)
    : config_(std::move(config))
{
    if (config_.command_uri.empty() || config_.timeout_ms == 0 || config_.timeout_ms > 60000)
        throw std::invalid_argument("REPL: provide URI and timeout between 1 and 60000 ms");
}

ServiceResult AdapterServiceServer::execute_zmq(const iit::advrf::Repl_cmd& request) const
{
    if (!request.IsInitialized())
        return {false, false, "Uninitialized REPL command"};
    bool sent = false;

    try
    {
        zmq::context_t context{1};
        // A fresh REQ socket per transaction prevents a late reply after a
        // timeout from being mistaken for the next command's acknowledgement.
        zmq::socket_t socket(context, ZMQ_REQ);
        socket.set(zmq::sockopt::linger, 0);
        socket.set(zmq::sockopt::sndtimeo, static_cast<int>(config_.timeout_ms));
        socket.set(zmq::sockopt::rcvtimeo, static_cast<int>(config_.timeout_ms));
        socket.connect(config_.command_uri);

        zmq::multipart_t outgoing;
        // The master pops the routing header first, then the Protobuf body.
        // EcReplCmd uses push_front twice; addstr appends, so order is explicit.
        outgoing.addstr(request.type() == iit::advrf::ECAT_MASTER_CMD ||
                                request.type() == iit::advrf::FOE_MASTER
                            ? "MASTER_CMD" : "ESC_CMD");
        outgoing.addstr(request.SerializeAsString());

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
            reply.cmd_type() != request.type() ||
            (reply.type() != iit::advrf::Cmd_reply::ACK &&
                reply.type() != iit::advrf::Cmd_reply::NACK))
        {
            return {false, true, "Malformed or unexpected master reply; command outcome unknown"};
        }

        *reply.mutable_request_id() = request.request_id();
        return {reply.type() == iit::advrf::Cmd_reply::ACK, false, reply.msg(), reply};
    }
    catch (const zmq::error_t& error)
    {
        return {false, sent, std::string("ZeroMQ: ") + error.what()};
    }
}
} // namespace advrf::middleware::adapters::service
