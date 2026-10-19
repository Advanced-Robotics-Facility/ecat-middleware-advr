include(CTest)

function(plugin_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME}
        ${TEST_SOURCE}
    )

    target_link_libraries(${TEST_NAME}
        PRIVATE
            advrf_middleware_core
            advrf_interfaces_dds::cyclonedds
            advrf_cyclonedds_plugin
    )

    add_test(
        NAME ${TEST_NAME}
        COMMAND $<TARGET_FILE:${TEST_NAME}>
    )
endfunction()



plugin_test(
    001_converter_001_fromdds
    001_converter/001_converter_001_fromdds.cpp
)

add_executable(004_config_qos 004_config_qos/004_config_qos.cpp)
target_link_libraries(004_config_qos PRIVATE advrf_cyclonedds_plugin)
add_test(NAME 004_config_qos COMMAND $<TARGET_FILE:004_config_qos>)

target_link_libraries(004_config_qos PRIVATE advrf_middleware_core_test_support)
