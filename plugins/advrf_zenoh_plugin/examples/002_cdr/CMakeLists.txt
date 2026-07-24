add_executable(ros2_compliant_pub
    002_pub.cpp
)

add_executable(ros2_compliant_sub
    002_sub.cpp
)

target_compile_features(ros2_compliant_pub PRIVATE cxx_std_17)
target_compile_features(ros2_compliant_sub PRIVATE cxx_std_17)

target_link_libraries(ros2_compliant_pub
    PRIVATE
        zenohcxx::zenohc
        fastcdr
        ecat_master_future
)

target_link_libraries(ros2_compliant_sub
    PRIVATE
        zenohcxx::zenohc
        fastcdr
        ecat_master_future
)


# Note: To run everything with ROS2, you must ->
#   a. Middleware side 
#       - Run the publisher 
#   b. ROS2 Side (rmw_cyclonedds_cpp)
#       - Start zenoh-bridge-ros2dds executable
#       - Run ros2 topic echo advrf/robot/imu advrf_interfaces/msg/Imu 
#
# For commuication between different PCs
# you need to change the endpoint in the publisher zenoh config