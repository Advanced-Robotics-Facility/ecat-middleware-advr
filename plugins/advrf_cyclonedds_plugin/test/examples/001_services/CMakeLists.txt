add_executable(001_services_sdo_get_request_server
    001_services_sdo_get_request_server.cpp
)

target_link_libraries(001_services_sdo_get_request_server
    PRIVATE
        advrf_cyclonedds_plugin
)

install(
    TARGETS 001_services_sdo_get_request_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(001_services_sdo_get_request_client
    001_services_sdo_get_request_client.cpp
)

target_link_libraries(001_services_sdo_get_request_client
    PRIVATE
        advrf_cyclonedds_plugin
)

install(
    TARGETS 001_services_sdo_get_request_client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


add_executable(001_services_cmd_server
    001_services_cmd_server.cpp
)

target_link_libraries(001_services_cmd_server
    PRIVATE
        advrf_cyclonedds_plugin
)

install(
    TARGETS 001_services_cmd_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)