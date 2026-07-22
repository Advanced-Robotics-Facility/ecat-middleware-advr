#include <zmq.hpp>

#include <string_view>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char** argv) {
    using namespace std::chrono_literals;

    zmq::context_t ctx {1};

    zmq::socket_t sock(ctx, zmq::socket_type::rep);
    sock.bind("tcp://*:5555");

    const std::string_view data {"World"};

    for (;;) {
        zmq::message_t request;

        sock.recv(request, zmq::recv_flags::none);
        std::cout << "Received: " << request.to_string() << '\n';

        std::this_thread::sleep_for(1s);

        sock.send(zmq::buffer(data), zmq::send_flags::none);
    }

    return 0;
}