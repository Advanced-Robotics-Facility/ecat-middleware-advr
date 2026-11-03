include(CTest)

function(plugin_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME}
        ${TEST_SOURCE}
    )

    target_link_libraries(${TEST_NAME}
        PRIVATE
            advrf_middleware_core
    )

    add_test(
        NAME ${TEST_NAME}
        COMMAND $<TARGET_FILE:${TEST_NAME}>
    )
endfunction()

plugin_test(
    001_adapters
    001_adapters/001_adapters.cpp
)


plugin_test(002_config 002_config/002_config.cpp)

target_link_libraries(002_config PRIVATE advrf_middleware_core_test_support)

plugin_test(003_adapter_service 003_adapter_service/003_adapter_service.cpp)
target_link_libraries(003_adapter_service PRIVATE ${ADVRF_ZMQ_LIBRARY})
set_tests_properties(003_adapter_service PROPERTIES TIMEOUT 10)
