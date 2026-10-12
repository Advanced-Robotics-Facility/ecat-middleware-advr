#include <publisher/zmq_publisher.hpp>
#include <subscriber/zmq_subscriber.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <exception>
#include <iostream>
#include <thread>

using namespace advrf::zmq_plugin;

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
        client_transport().initialize(false);

        // One shared context with one ZeroMQ I/O thread.
        zmq::context_t context{1};

        ZmqPublisher publisher{context,"tcp://*:9001"};

        ZmqSubscriber subscriber{context,"tcp://*:9002"};

        std::thread publisher_thread{
            [&publisher] {
                publisher.run();
            }
        };

        /*
        std::thread subscriber_thread{
            [&subscriber] {
                subscriber.run();
            }
        };
        */

        // Wait until Ctrl+C or SIGTERM.
        while (!stop_requested) {
            std::this_thread::sleep_for(std::chrono::milliseconds{100});
        }

        // These functions should only set atomic stop flags.
        publisher.stop();
        subscriber.stop();

        if (publisher_thread.joinable()) {
            publisher_thread.join();
        }

        /*
        if (subscriber_thread.joinable()) {
            subscriber_thread.join();
        }
        */
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
