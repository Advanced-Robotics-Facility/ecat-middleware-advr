add_executable(002_parameters_parameter_client
    002_parameters_parameter_client.cpp
)

target_link_libraries(002_parameters_parameter_client
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_interfaces_dds
        advrf_interfaces_protobuf
)

install(
    TARGETS 002_parameters_parameter_client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(002_parameters_parameter_server
    002_parameters_parameter_server.cpp
)

target_link_libraries(002_parameters_parameter_server
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_interfaces_dds
        advrf_interfaces_protobuf
)

install(
    TARGETS 002_parameters_parameter_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)