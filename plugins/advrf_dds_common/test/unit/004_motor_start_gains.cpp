#include <stdexcept>
#include <advrf_ros2_interfaces/msg/CtrlCmd.hpp>
#include "advrf_dds_common/converter/converter.hpp"

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(#condition); } while (false)

int main()
{
    advrf_ros2_interfaces::msg::dds_::CtrlCmd_ input;
    input.type() = 11;
    input.board_id() = 1;
    input.value() = 59;
    iit::advrf::Ctrl_cmd output;
    advrf::dds_common::convert::protobuf::from_dds(input, output);
    CHECK(output.type() == iit::advrf::Ctrl_cmd::CTRL_CMD_START);
    CHECK(output.board_id() == 1);
    CHECK(output.has_value() && output.value() == 59);
    CHECK(!output.has_gains());

    // Explicit gains, including an intentional all-zero set, remain supported.
    input.gains().type() = 59;
    input.gains().pos_kp() = 0.1;
    advrf::dds_common::convert::protobuf::from_dds(input, output);
    CHECK(output.has_gains());
    CHECK(output.gains().type() == iit::advrf::Gains::POSITION);
    CHECK(output.gains().pos_kp() == 0.1);
    input.gains().pos_kp() = 0;
    advrf::dds_common::convert::protobuf::from_dds(input, output);
    CHECK(output.has_gains() && output.gains().pos_kp() == 0);

    // Reused protobuf objects must not retain an earlier explicit Gains message.
    input.gains().type() = 0;
    advrf::dds_common::convert::protobuf::from_dds(input, output);
    CHECK(!output.has_gains());
    input.value() = 212;
    advrf::dds_common::convert::protobuf::from_dds(input, output);
    CHECK(output.value() == 212 && !output.has_gains());
    input.type() = 12;
    advrf::dds_common::convert::protobuf::from_dds(input, output);
    CHECK(output.type() == iit::advrf::Ctrl_cmd::CTRL_CMD_STOP);
}
