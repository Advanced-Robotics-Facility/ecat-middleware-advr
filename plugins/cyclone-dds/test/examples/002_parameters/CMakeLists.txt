add_executable(002_parameters_parameter-client
    002_parameters_parameter-client.cpp
)

target_link_libraries(002_parameters_parameter-client
    PRIVATE
        ecat_plugin_cyclonedds
)

add_executable(002_parameters_parameter-server
    002_parameters_parameter-server.cpp
)

target_link_libraries(002_parameters_parameter-server
    PRIVATE
        ecat_plugin_cyclonedds
)


add_executable(tmp-test
    tmp-test.cpp
)

target_link_libraries(tmp-test
    PRIVATE
        ecat_plugin_cyclonedds
        advrf_interfaces_dds
)
