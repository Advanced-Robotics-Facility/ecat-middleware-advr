#include <iostream>
#include <vector>

#include <dds/dds.hpp>

#include "advrf_cyclonedds_plugin/parameter/parameter_make_get.hpp"
#include "advrf_cyclonedds_plugin/service.hpp"

#include <advrf_interfaces/srv/SdoGetRequest.hpp>

constexpr int DOMAIN_ID = 42;

template<typename T>
void print_vector(const std::vector<T>& values)
{
    std::cout << "[";
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (i)
            std::cout << ", ";
        std::cout << values[i];
    }
    std::cout << "]";
}

inline void print_vector(const std::vector<uint8_t>& values)
{
    std::cout << "[";
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (i)
            std::cout << ", ";
        std::cout << static_cast<int>(values[i]);
    }
    std::cout << "]";
}

inline void print_vector(const std::vector<bool>& values)
{
    std::cout << "[";
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (i)
            std::cout << ", ";
        std::cout << std::boolalpha << static_cast<bool>(values[i]);
    }
    std::cout << "]";
}

void print_parameter(const rcl_interfaces::msg::dds_::Parameter_& parameter)
{
    std::cout << parameter.name() << " = ";

    switch (parameter.value().type())
    {
        case rcl_interfaces::msg::dds_::PARAMETER_BOOL:
            std::cout << std::boolalpha
                      << get_parameter<bool>(parameter);
            break;

        case rcl_interfaces::msg::dds_::PARAMETER_INTEGER:
            std::cout << get_parameter<int64_t>(parameter);
            break;

        case rcl_interfaces::msg::dds_::PARAMETER_DOUBLE:
            std::cout << get_parameter<double>(parameter);
            break;

        case rcl_interfaces::msg::dds_::PARAMETER_STRING:
            std::cout << get_parameter<std::string>(parameter);
            break;

        case rcl_interfaces::msg::dds_::PARAMETER_BYTE_ARRAY:
            print_vector(get_parameter<std::vector<uint8_t>>(parameter));
            break;

        case rcl_interfaces::msg::dds_::PARAMETER_BOOL_ARRAY:
            print_vector(get_parameter<std::vector<bool>>(parameter));
            break;

        case rcl_interfaces::msg::dds_::PARAMETER_INTEGER_ARRAY:
            print_vector(get_parameter<std::vector<int64_t>>(parameter));
            break;

        case rcl_interfaces::msg::dds_::PARAMETER_DOUBLE_ARRAY:
            print_vector(get_parameter<std::vector<double>>(parameter));
            break;

        case rcl_interfaces::msg::dds_::PARAMETER_STRING_ARRAY:
            print_vector(get_parameter<std::vector<std::string>>(parameter));
            break;

        default:
            std::cout << "<unsupported>";
            break;
    }

    std::cout << '\n';
}

int main()
{
    dds::domain::DomainParticipant participant(DOMAIN_ID);

    ServiceClient<
        std_srvs::srv::dds_::SdoGet_Request_,
        std_srvs::srv::dds_::SdoGet_Response_> client(
            participant,
            "rq/advrf/spot/sdoGetRequest",
            "rr/advrf/spot/sdoGetReply");

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std_srvs::srv::dds_::SdoGet_Request_ request;
    request.sub_request(std_srvs::srv::dds_::SUB_REQUEST_ALL);

    auto response = client.call(request);

    if (!response)
    {
        std::cerr << "Request timeout.\n";
        return 1;
    }

    if (!response->success())
    {
        std::cerr << "Server returned an error.\n";
        return 1;
    }

    std::cout << "Received "
              << response->parameters().size()
              << " parameters\n\n";

    for (const auto& parameter : response->parameters())
    {
        print_parameter(parameter);
    }

    return 0;
}