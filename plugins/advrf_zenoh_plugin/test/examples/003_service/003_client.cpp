#include "zenoh.hxx"
using namespace zenoh;

int main(int argc, char **argv) {
   Config config = Config::create_default();
   auto session = Session::open(std::move(config));

   auto replies = session.get(KeyExpr("demo/example/simple"), "", channels::FifoChannel(16));
   while (true) {
      auto res = replies.recv();
      auto* reply = std::get_if<zenoh::Reply>(&res);
      if (reply == nullptr) break;
      if (reply->is_ok()) {
         const Sample& sample = reply->get_ok();
         std::cout << "Received ('" << sample.get_keyexpr().as_string_view() << "' : '"
                   << sample.get_payload().as_string() << "')\n";
      } else {
         const ReplyError& error = reply->get_err();
         std::cout << "Received an error :"
                   << error.get_payload().as_string() << "\n";
      }

   }

   std::cout << "Ok.\n";
}