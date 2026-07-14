#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "advrf_cyclonedds_plugin/parameter/parameter_make_get.hpp"

template<typename T>
void test_parameter(const T& value)
{
    auto msg = make_parameter_value(value);
    auto out = get_parameter_value<T>(msg);
    assert(out == value);
}
int main()
{
    test_parameter(true);
    test_parameter(false);
    test_parameter(42);
    test_parameter(3.14);
    test_parameter(std::string("hello"));
    test_parameter(std::vector<uint8_t>{1,2,3});
    test_parameter(std::vector<bool>{true,false});
    test_parameter(std::vector<int64_t>{1,2,3});
    test_parameter(std::vector<double>{1.1,2.2});
    test_parameter(std::vector<std::string>{"foo","bar"});
    
    auto msg = make_parameter_value(42);
    try
    {
        [[maybe_unused]] auto value = get_parameter_value<double>(msg);
        assert(false && "Expected exception was not thrown.");
    }
    catch (const std::runtime_error&)
    {
        // OK
    }
    
    std::cout << "All ParameterValue tests passed." << std::endl;
    return 0;
}