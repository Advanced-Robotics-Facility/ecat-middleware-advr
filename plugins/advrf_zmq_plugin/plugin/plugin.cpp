#include <publisher/zmq_publisher.hpp>
#include <subscriber/zmq_subscriber.hpp>
#include <advrf_middleware_core/plugin/plugin_exec.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <exception>
#include <iostream>

using namespace advrf::zmq_plugin;
using namespace advrf::middleware::config;
namespace {

volatile std::sig_atomic_t stop_requested = 0;

extern "C" void signal_handler(int)
{
    stop_requested = 1;
}

} // namespace

int main(int argc, char** argv)
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {

        // One shared context with one ZeroMQ I/O thread.
        zmq::context_t context{1};

        auto publisher_mng = std::make_shared<ZmqPublisherManager>();
        publisher_mng->setup(context, "tcp://*:9001");
        publisher_mng->start();

        auto subscriber = std::make_shared<ZmqSubscriber>(context,"tcp://*:9002");
        subscriber->start();

        advrf::middleware::plugin::PluginExec exec;
        exec.register_adapter(advrf::middleware::plugin::Process{
            "zmq_adapter_publishers", publisher_mng, std::chrono::microseconds{3000}});

        exec.register_adapter(advrf::middleware::plugin::Process{
            "zmq_adapter_subscribers", subscriber, std::chrono::microseconds{0}});

        exec.start();

    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
