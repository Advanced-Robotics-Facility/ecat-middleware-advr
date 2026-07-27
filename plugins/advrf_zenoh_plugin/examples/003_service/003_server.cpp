#include "zenoh.hxx"
#include <iostream>
using namespace zenoh;

int main(int argc, char **argv) {
   auto queryable_keyexpr = KeyExpr("demo/example/simple");
   Config config = Config::create_default();
   auto session = Session::open(std::move(config));
   auto queryable = session.declare_queryable(
         queryable_keyexpr,
         [&queryable_keyexpr](const Query& query) {
            std::cout << "Received Query '"
                     << query.get_keyexpr().as_string_view()
                     << query.get_parameters() << '\n';
            query.reply(queryable_keyexpr, zenoh::Bytes("42"));
      },
      closures::none
   );
   char c = getchar();
}
