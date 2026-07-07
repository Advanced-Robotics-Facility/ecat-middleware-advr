add_executable(sdo_get_request_server
    sdo_get_request_server.cpp
)

target_link_libraries(sdo_get_request_server
    PRIVATE
        ecat_plugin_cyclonedds
)

add_executable(sdo_get_request_client
    sdo_get_request_client.cpp
)

target_link_libraries(sdo_get_request_client
    PRIVATE
        ecat_plugin_cyclonedds
)
