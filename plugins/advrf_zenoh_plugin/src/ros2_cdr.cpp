#include "advrf_zenoh_plugin/serialization/ros2_cdr.hpp"

#include <cstdint>
#include <string>
#include <utility>

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/exceptions/Exception.h>

namespace advrf::zenoh_plugin::ros2_cdr
{

bool serialize(const iit::advrf::Header& header,
               const iit::advrf::ImuVN_rx_pdo& imu,
               std::vector<std::uint8_t>& payload)
{
    payload.clear();

    if (!header.has_stamp()) return false;
    const auto& stamp = header.stamp();

    try
    {
        eprosima::fastcdr::FastBuffer buffer;
        eprosima::fastcdr::Cdr cdr(
            buffer,
            eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::CdrVersion::XCDRv1
        );

        cdr.serialize_encapsulation();

        // std_msgs::msg::dds_::Header_
        cdr << static_cast<std::int32_t>(stamp.sec());
        cdr << static_cast<std::uint32_t>(stamp.nsec());
        cdr << header.str_id();

        // geometry_msgs::msg::dds_::Vector3_ linear_acceleration
        cdr << static_cast<double>(imu.x_acc());
        cdr << static_cast<double>(imu.y_acc());
        cdr << static_cast<double>(imu.z_acc());

        // geometry_msgs::msg::dds_::Vector3_ angular_velocity
        cdr << static_cast<double>(imu.x_rate());
        cdr << static_cast<double>(imu.y_rate());
        cdr << static_cast<double>(imu.z_rate());

        // geometry_msgs::msg::dds_::Quaternion_ orientation
        cdr << static_cast<double>(imu.x_quat());
        cdr << static_cast<double>(imu.y_quat());
        cdr << static_cast<double>(imu.z_quat());
        cdr << static_cast<double>(imu.w_quat());

        cdr << static_cast<std::uint32_t>(imu.imu_ts());
        cdr << static_cast<std::uint32_t>(imu.temperature());
        cdr << static_cast<std::uint32_t>(imu.digital_in());
        cdr << static_cast<std::uint32_t>(imu.fault());
        cdr << static_cast<std::uint32_t>(imu.rtt());

        const auto size = cdr.get_serialized_data_length();
        const auto* data = reinterpret_cast<const std::uint8_t*>(buffer.getBuffer());
        payload.assign(data, data + size);
        return true;
    }
    catch (const eprosima::fastcdr::exception::Exception&)
    {
        payload.clear();
        return false;
    }
}

bool deserialize(const zenoh::Bytes& payload,
                 iit::advrf::Header& header,
                 iit::advrf::ImuVN_rx_pdo& imu)
{
    auto bytes = payload.as_vector();
    eprosima::fastcdr::FastBuffer buffer(reinterpret_cast<char*>(bytes.data()), bytes.size());
    eprosima::fastcdr::Cdr cdr(buffer);

    try
    {
        cdr.read_encapsulation();

        std::int32_t stamp_sec{};
        std::uint32_t stamp_nanosec{};
        std::string frame_id;

        double ax{}, ay{}, az{};
        double wx{}, wy{}, wz{};
        double qx{}, qy{}, qz{}, qw{};

        std::uint32_t imu_ts{};
        std::uint32_t temperature{};
        std::uint32_t digital_in{};
        std::uint32_t fault{};
        std::uint32_t rtt{};

        cdr >> stamp_sec;
        cdr >> stamp_nanosec;
        cdr >> frame_id;
 
        cdr >> ax >> ay >> az;
        cdr >> wx >> wy >> wz;
        cdr >> qx >> qy >> qz >> qw;

        cdr >> imu_ts;
        cdr >> temperature;
        cdr >> digital_in;
        cdr >> fault;
        cdr >> rtt;

        iit::advrf::Header decoded_header;
        decoded_header.mutable_stamp()->set_sec(stamp_sec);
        decoded_header.mutable_stamp()->set_nsec(static_cast<std::int32_t>(stamp_nanosec));
        decoded_header.set_str_id(std::move(frame_id));

        iit::advrf::ImuVN_rx_pdo decoded_imu;
        decoded_imu.set_x_acc(static_cast<float>(ax));
        decoded_imu.set_y_acc(static_cast<float>(ay));
        decoded_imu.set_z_acc(static_cast<float>(az));
        decoded_imu.set_x_rate(static_cast<float>(wx));
        decoded_imu.set_y_rate(static_cast<float>(wy));
        decoded_imu.set_z_rate(static_cast<float>(wz));
        decoded_imu.set_x_quat(static_cast<float>(qx));
        decoded_imu.set_y_quat(static_cast<float>(qy));
        decoded_imu.set_z_quat(static_cast<float>(qz));
        decoded_imu.set_w_quat(static_cast<float>(qw));
        decoded_imu.set_imu_ts(imu_ts);
        decoded_imu.set_temperature(temperature);
        decoded_imu.set_digital_in(digital_in);
        decoded_imu.set_fault(fault);
        decoded_imu.set_rtt(rtt);

        header = std::move(decoded_header);
        imu = std::move(decoded_imu);
        return true;
    }
    catch (const eprosima::fastcdr::exception::Exception&)
    {
        return false;
    }
}

} 
