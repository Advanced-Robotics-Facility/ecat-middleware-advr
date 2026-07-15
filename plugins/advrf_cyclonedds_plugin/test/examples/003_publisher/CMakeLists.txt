add_executable(publisher
    publisher.cpp
)

target_compile_features(publisher
    PRIVATE
        cxx_std_17
)

target_link_libraries(publisher
    PRIVATE
        advrf_cyclonedds_plugin
        yaml-cpp::yaml-cpp
)

target_compile_definitions(publisher PRIVATE
    ROBOT_CONFIG_DIR="${ecat_master_future_CONFIG_DIR}/robot.yaml"
)

add_test(
    NAME publisher
    COMMAND publisher
)