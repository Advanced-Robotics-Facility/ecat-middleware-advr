#include <advrf_middleware_core/adapters_mode/motor_control.hpp>

#include <atomic>
#include <future>
#include <limits>
#include <protobuf/repl_cmd.pb.h>
#include <stdexcept>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>

using namespace advrf::middleware::adapters_mode;
#define CHECK(value)                                                                               \
    do                                                                                             \
    {                                                                                              \
        if (!(value))                                                                              \
            throw std::runtime_error(#value);                                                      \
    } while (false)

int main()
{
    std::promise<std::string> endpoint;
    std::promise<void> completed;
    auto completion = completed.get_future();
    std::thread server([&]
    {
        try
        {
            zmq::context_t context(1);
            zmq::socket_t socket(context, ZMQ_REP);
            socket.set(zmq::sockopt::linger, 0);
            socket.set(zmq::sockopt::rcvtimeo, 3000);
            socket.bind("tcp://127.0.0.1:*");
            endpoint.set_value(socket.get(zmq::sockopt::last_endpoint));

            for (int index = 0; index < 7; ++index)
            {
                zmq::multipart_t frames;
                CHECK(frames.recv(socket));
                CHECK(frames.size() == 2);
                CHECK(frames.popstr() == "ESC_CMD");
                iit::advrf::Repl_cmd command;
                CHECK(command.ParseFromString(frames.popstr()));
                CHECK(frames.empty());
                CHECK(command.type() == iit::advrf::CTRL_CMD);
                CHECK(command.ctrl_cmd().board_id() == 42);
                if (index == 0 || index == 5 || index == 6)
                {
                    CHECK(command.ctrl_cmd().type() == iit::advrf::Ctrl_cmd::CTRL_CMD_START);
                    CHECK(command.ctrl_cmd().value() == 59);
                    CHECK(command.ctrl_cmd().gains().type() == iit::advrf::Gains_Type_POSITION);
                    CHECK(command.ctrl_cmd().gains().pos_kp() == 1);
                    CHECK(command.ctrl_cmd().gains().pos_kd() == 3);
                    CHECK(command.ctrl_cmd().gains().tor_ki() == 2);
                }
                if (index == 1 || index == 2)
                {
                    CHECK(command.ctrl_cmd().type() == iit::advrf::Ctrl_cmd::CTRL_CMD_STOP);
                    CHECK(!command.ctrl_cmd().has_gains());
                }
                if (index == 3)
                {
                    CHECK(command.ctrl_cmd().type() == iit::advrf::Ctrl_cmd::CTRL_SET_GAINS);
                }

                if (index == 6)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    break; // Lost reply, no retry expected.
                }
                if (index == 4)
                {
                    socket.send(zmq::buffer("invalid"), zmq::send_flags::none);
                    continue;
                }
                iit::advrf::Cmd_reply reply;
                reply.set_type(
                    index == 5 ? iit::advrf::Cmd_reply::NACK : iit::advrf::Cmd_reply::ACK);
                reply.set_cmd_type(iit::advrf::CTRL_CMD);
                reply.set_msg(index == 5 ? "Rejected by mock" : "Mock ACK");
                socket.send(zmq::buffer(reply.SerializeAsString()), zmq::send_flags::none);
            }
            completed.set_value();
        }
        catch (...)
        {
            try
            {
                endpoint.set_exception(std::current_exception());
            }
            catch (...)
            {
            }
            completed.set_exception(std::current_exception());
        }
    });

    try
    {
        MotorControlConfig config;
        config.enabled = true;
        config.command_uri = endpoint.get_future().get();
        config.timeout_ms = 100;
        config.allowed_motor_ids = {42};
        config.advrf_motor_ids = {42};
        MotorControlAdapter adapter(config);
        MotorControlRequest request;
        request.motor_id = 42;
        request.command = MotorCommand::Start;
        request.control_mode = 59;
        request.gains = {1, 2, 3, 4, 5};
        CHECK(!adapter.execute(request).success);
        CHECK(adapter.start());
        CHECK(adapter.is_ok());
        auto invalid = request;
        invalid.motor_id = 12;
        CHECK(!adapter.execute(invalid).success);
        invalid = request;
        invalid.command = static_cast<MotorCommand>(255);
        CHECK(!adapter.execute(invalid).success);
        invalid = request;
        invalid.control_mode = UINT32_MAX;
        CHECK(!adapter.execute(invalid).success);
        invalid = request;
        invalid.gains[0] = std::numeric_limits<float>::quiet_NaN();
        CHECK(!adapter.execute(invalid).success);
        CHECK(adapter.execute(request).success);
        request.command = MotorCommand::Stop;
        CHECK(adapter.execute(request).success);
        request.command = MotorCommand::Shutdown;
        CHECK(adapter.execute(request).success);
        request.command = MotorCommand::SetGains;
        CHECK(adapter.execute(request).success);
        request.command = MotorCommand::Start;
        auto reply = adapter.execute(request);
        CHECK(!reply.success && reply.outcome_unknown);
        reply = adapter.execute(request);
        CHECK(!reply.success && !reply.outcome_unknown && reply.message == "Rejected by mock");
        reply = adapter.execute(request);
        CHECK(!reply.success && reply.outcome_unknown);
        adapter.close();
        CHECK(!adapter.is_ok());
        server.join();
        completion.get();
        return 0;
    }
    catch (...)
    {
        if (server.joinable())
        {
            server.join();
        }
        throw;
    }
}
