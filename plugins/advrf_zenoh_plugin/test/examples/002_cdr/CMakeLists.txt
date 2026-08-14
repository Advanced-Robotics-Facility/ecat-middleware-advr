add_executable(002_ros2_pub
    002_ros2_pub.cpp
)

target_compile_features(002_ros2_pub 
    PRIVATE 
        cxx_std_17
)

target_link_libraries(002_ros2_pub
    PRIVATE
        zenohcxx::zenohc
        fastcdr
        advrf_zenoh_plugin
)

install(
    TARGETS 002_ros2_pub
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---

add_executable(002_ros2_sub
    002_ros2_sub.cpp
)

target_compile_features(002_ros2_sub 
    PRIVATE 
        cxx_std_17
)

target_link_libraries(002_ros2_sub
    PRIVATE
        zenohcxx::zenohc
        fastcdr
        advrf_zenoh_plugin
)

install(
    TARGETS 002_ros2_sub
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
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
