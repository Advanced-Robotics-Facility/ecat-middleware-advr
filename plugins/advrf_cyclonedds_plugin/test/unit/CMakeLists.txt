function(plugin_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME}
        ${TEST_SOURCE}
    )

    target_link_libraries(${TEST_NAME}
        PRIVATE
            advrf_cyclonedds_plugin
            advrf_interfaces_dds
            advrf_interfaces_protobuf
    )

    add_test(
        NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
    )
endfunction()

plugin_test(
    001_sdo-request_001_parameters
    001_sdo-request/001_sdo-request_001_parameters.cpp
)

plugin_test(
    002_parameter-server_001
    002_parameter-server/002_parameter-server_001.cpp
)