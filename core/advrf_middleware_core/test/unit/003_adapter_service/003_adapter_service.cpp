#include <advrf_middleware_core/adapters/adapter_service.hpp>
#include <future>
#include <thread>
#include <stdexcept>
#include <zmq.hpp>
#include <zmq_addon.hpp>

using namespace advrf::middleware::adapters::service;
#define CHECK(value) do { if (!(value)) throw std::runtime_error(#value); } while (false)

int main()
{
    std::promise<std::string> endpoint;
    auto server = std::async(std::launch::async, [&] {
        try {
            zmq::context_t context(1);
            zmq::socket_t socket(context, ZMQ_REP);
            socket.set(zmq::sockopt::linger, 0);
            socket.set(zmq::sockopt::rcvtimeo, 3000);
            socket.bind("tcp://127.0.0.1:*");
            endpoint.set_value(socket.get(zmq::sockopt::last_endpoint));
            for (int index = 0; index < 6; ++index) {
                zmq::multipart_t frames;
                CHECK(frames.recv(socket));
                CHECK(frames.size() == 2);
                CHECK(frames.popstr() == (index == 3 ? "MASTER_CMD" : "ESC_CMD"));
                iit::advrf::Repl_cmd request;
                CHECK(request.ParseFromString(frames.popstr()));
                if (index != 3) {
                    CHECK(request.type() == iit::advrf::CTRL_CMD);
                    CHECK(request.ctrl_cmd().board_id() == 42);
                    CHECK(request.ctrl_cmd().type() == (index == 1
                        ? iit::advrf::Ctrl_cmd::CTRL_CMD_STOP
                        : iit::advrf::Ctrl_cmd::CTRL_CMD_START));
                    if (index == 0) {
                        CHECK(request.ctrl_cmd().value() == 59);
                        CHECK(request.ctrl_cmd().gains().pos_kp() == 1);
                    }
                }
                if (index == 5) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    break;
                }
                if (index == 4) {
                    socket.send(zmq::buffer("invalid"), zmq::send_flags::none);
                    continue;
                }
                iit::advrf::Cmd_reply reply;
                reply.set_type(index == 2 ? iit::advrf::Cmd_reply::NACK : iit::advrf::Cmd_reply::ACK);
                reply.set_cmd_type(request.type());
                reply.set_msg(index == 2 ? "Rejected by mock" : "Mock ACK");
                socket.send(zmq::buffer(reply.SerializeAsString()), zmq::send_flags::none);
            }
        } catch (...) {
            try { endpoint.set_exception(std::current_exception()); } catch (...) {}
            throw;
        }
    });

    AdapterServiceServer adapter({ServiceTransport::Zmq, endpoint.get_future().get(), 100});
    iit::advrf::Repl_cmd request;
    request.set_type(iit::advrf::CTRL_CMD);
    request.mutable_request_id()->set_guid(456);
    request.mutable_request_id()->set_seq(123);
    auto* ctrl = request.mutable_ctrl_cmd();
    ctrl->set_board_id(42);
    ctrl->set_type(iit::advrf::Ctrl_cmd::CTRL_CMD_START);
    ctrl->set_value(59);
    ctrl->mutable_gains()->set_type(iit::advrf::Gains_Type_POSITION);
    ctrl->mutable_gains()->set_pos_kp(1);
    CHECK(adapter.process_request(request).type() == iit::advrf::Cmd_reply::NACK);
    CHECK(adapter.start());
    CHECK(adapter.is_ok());
    auto reply = adapter.process_request(request);
    CHECK(reply.type() == iit::advrf::Cmd_reply::ACK);
    CHECK(reply.request_id().guid() == 456 && reply.request_id().seq() == 123);
    ctrl->set_type(iit::advrf::Ctrl_cmd::CTRL_CMD_STOP);
    CHECK(adapter.process_request(request).type() == iit::advrf::Cmd_reply::ACK);
    ctrl->set_type(iit::advrf::Ctrl_cmd::CTRL_CMD_START);
    auto result = adapter.process_request_result(request);
    CHECK(!result.success && !result.outcome_unknown && result.message == "Rejected by mock");
    auto master_request = request;
    master_request.set_type(iit::advrf::ECAT_MASTER_CMD);
    master_request.mutable_ecat_master_cmd()->set_type(iit::advrf::Ecat_Master_cmd::GET_SLAVES_DESCR);
    CHECK(adapter.process_request(master_request).type() == iit::advrf::Cmd_reply::ACK);
    result = adapter.process_request_result(request);
    CHECK(!result.success && result.outcome_unknown);
    result = adapter.process_request_result(request);
    CHECK(!result.success && result.outcome_unknown);
    adapter.close();
    CHECK(!adapter.is_ok());
    server.get();
}
