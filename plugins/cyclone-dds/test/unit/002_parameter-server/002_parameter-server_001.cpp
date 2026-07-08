#include <cassert>
#include <iostream>
#include <advrf_interfaces/msg/Enums.hpp>

#include "parameter/parameter_registry.hpp"


int main()
{
    ParameterRegistry server;

    server.declare("kp",12.5);
    server.declare("ki",0.2);
    server.declare("enabled",true);

    assert(server.size()==3);

    assert(server.has("kp"));
    assert(server.has("ki"));
    assert(server.has("enabled"));

    assert(server.get<double>("kp")==12.5);
    assert(server.get<double>("ki")==0.2);
    assert(server.get<bool>("enabled"));

    server.set("kp",15.0);

    assert(server.get<double>("kp")==15.0);

    server.remove("ki");

    assert(server.size()==2);
    assert(!server.has("ki"));


    server.declare("positive_value",1.0, {
        .validator = [](const double value)
        {
            return value > 0.0;
        }
    });

    using ParameterResult = advrf_interfaces::msg::dds_::enums_::ParameterResult_;
    assert(server.set("positive_value",2.0) == ParameterResult::Success);
    assert(server.set("positive_value",-1.0) == ParameterResult::ValidationFailed);

    std::cout<<"ParameterServer test passed"<<std::endl;
}