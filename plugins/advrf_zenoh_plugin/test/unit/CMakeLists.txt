include(CTest)

function(plugin_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME}
        ${TEST_SOURCE}
    )

    target_link_libraries(${TEST_NAME}
        PRIVATE
            advrf_middleware_core
            advrf_zenoh_plugin
    )

    add_test(
        NAME ${TEST_NAME}
        COMMAND $<TARGET_FILE:${TEST_NAME}>
    )
endfunction()

plugin_test(
    001_proto_serialization
    001_proto_serialization/001_proto_serialization.cpp
)