#include <advrf_cyclonedds_plugin/adapters/dds_adapter_motor_control.hpp>

#include <atomic>
#include <future>
#include <iostream>
#include <protobuf/repl_cmd.pb.h>
#include <stdexcept>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>

using namespace advrf::cyclonedds_plugin;
using namespace advrf::middleware::adapters_mode;
#define CHECK(value)                                                                               \
    do                                                                                             \
    {                                                                                              \
        if (!(value))                                                                              \
            throw std::runtime_error(#value);                                                      \
    } while (false)

int main(int argc, char** argv)
{
    const bool python_client = argc > 1 && (std::string(argv[1]) == "--serve-python" ||
                                               std::string(argv[1]) == "--serve-ros");
    const int expected_commands = python_client ? 4 : 3;
    std::promise<std::string> endpoint;
    std::promise<void> done;
    auto result = done.get_future();
    std::atomic<int> count{0};
    std::thread master([&]
    {
        try
        {
            zmq::context_t context(1);
            zmq::socket_t socket(context, ZMQ_REP);
            socket.set(zmq::sockopt::linger, 0);
            socket.set(zmq::sockopt::rcvtimeo, 15000);
            socket.bind("tcp://127.0.0.1:*");
            endpoint.set_value(socket.get(zmq::sockopt::last_endpoint));
            for (int i = 0; i < expected_commands; ++i)
            {
                zmq::multipart_t frames;
                CHECK(frames.recv(socket));
                CHECK(frames.size() == 2 && frames.popstr() == "ESC_CMD");
                iit::advrf::Repl_cmd command;
                CHECK(command.ParseFromString(frames.popstr()));
                CHECK(command.ctrl_cmd().board_id() == 42);
                const auto expected_type =
                    i == 0 ? iit::advrf::Ctrl_cmd::CTRL_CMD_START
                           : (python_client && i == 1 ? iit::advrf::Ctrl_cmd::CTRL_SET_GAINS
                                                      : iit::advrf::Ctrl_cmd::CTRL_CMD_STOP);
                CHECK(command.ctrl_cmd().type() == expected_type);
                if (i == 0)
                {
                    CHECK(command.ctrl_cmd().value() == 59);
                    CHECK(command.ctrl_cmd().gains().pos_kp() == 0.1f);
                }
                iit::advrf::Cmd_reply reply;
                reply.set_type(iit::advrf::Cmd_reply::ACK);
                reply.set_cmd_type(iit::advrf::CTRL_CMD);
                reply.set_msg("Mock master ACK");
                socket.send(zmq::buffer(reply.SerializeAsString()), zmq::send_flags::none);
                ++count;
            }
            done.set_value();
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
            done.set_exception(std::current_exception());
        }
    });

    try
    {
        MotorControlConfig config;
        config.enabled = true;
        config.command_uri = endpoint.get_future().get();
        config.timeout_ms = 500;
        config.allowed_motor_ids = {42};
        dds::domain::DomainParticipant participant(217);
        advrf::middleware::config::ConfigTopics topics({"advrf", "motor_service_test"});
        DDSAdapterMotorControl adapter(topics, participant, config);
        CHECK(adapter.start());

        if (argc > 1)
        {
            std::cout << "Mock motor service ready on domain 217" << std::endl;
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
            while (count < expected_commands && std::chrono::steady_clock::now() < deadline)
            {
                adapter.spin_once();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            CHECK(count == expected_commands);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        else
        {
            dds::pub::Publisher publisher(participant);
            dds::sub::Subscriber subscriber(participant);
            dds::topic::Topic<RosMotorRequestDDS> request_topic(
                participant, topics.service.motorControlRosRequest());
            dds::topic::Topic<RosMotorReplyDDS> reply_topic(
                participant, topics.service.motorControlRosReply());
            auto wq = publisher.default_datawriter_qos();
            wq << dds::core::policy::Reliability::Reliable();
            auto rq = subscriber.default_datareader_qos();
            rq << dds::core::policy::Reliability::Reliable();
            dds::pub::DataWriter<RosMotorRequestDDS> writer(publisher, request_topic, wq);
            dds::sub::DataReader<RosMotorReplyDDS> reader(subscriber, reply_topic, rq);
            const auto discovery = std::chrono::steady_clock::now() + std::chrono::seconds(3);
            while ((writer.publication_matched_status().current_count() == 0 ||
                       reader.subscription_matched_status().current_count() == 0) &&
                   std::chrono::steady_clock::now() < discovery)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            const auto call = [&](const RosMotorRequestDDS& request)
            {
                writer.write(request);
                const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
                while (std::chrono::steady_clock::now() < deadline)
                {
                    adapter.spin_once();
                    for (const auto& sample : reader.take())
                    {
                        if (sample.info().valid() &&
                            sample.data().request_id() == request.request_id())
                        {
                            return sample.data();
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                }
                throw std::runtime_error("DDS service timeout");
            };
            RosMotorRequestDDS request;
            request.request_id().guid(123);
            request.request_id().seq(1);
            request.motor_id(42);
            request.command(1);
            request.control_mode(59);
            request.gains({0.1f, 0, 0, 0, 0});
            CHECK(call(request).success());
            CHECK(count == 1);
            CHECK(call(request).success()); // Duplicate must not execute again.
            CHECK(count == 1);
            request.request_id().seq(2);
            request.motor_id(99);
            CHECK(!call(request).success());
            CHECK(count == 1);
            request.request_id().seq(3);
            request.motor_id(42);
            request.command(2);
            CHECK(call(request).success());
            request.request_id().seq(4);
            request.command(0);
            CHECK(call(request).success());
            CHECK(count == 3);
        }
        adapter.close();
        master.join();
        result.get();
        return 0;
    }
    catch (...)
    {
        if (master.joinable())
        {
            master.join();
        }
        throw;
    }
}
