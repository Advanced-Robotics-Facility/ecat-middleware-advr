add_executable(002_parameters_parameter-client
    002_parameters_parameter-client.cpp
)

target_link_libraries(002_parameters_parameter-client
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_interfaces_dds
        advrf_interfaces_protobuf
)

add_executable(002_parameters_parameter-server
    002_parameters_parameter-server.cpp
)

target_link_libraries(002_parameters_parameter-server
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_interfaces_dds
        advrf_interfaces_protobuf
)


add_executable(tmp-test
    tmp-test.cpp
)

target_link_libraries(tmp-test
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_interfaces_dds
        advrf_interfaces_protobuf
)
