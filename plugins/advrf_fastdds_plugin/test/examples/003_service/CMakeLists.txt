add_executable(003_service_repl_client
    003_service_repl_client.cpp
)

target_compile_features(003_service_repl_client
    PRIVATE
        cxx_std_17
)

target_link_libraries(003_service_repl_client
    PRIVATE
        advrf_fastdds_plugin
        advrf_interfaces_dds::fastdds
        advrf_middleware_core
        advrf_config
)



install(
    TARGETS 003_service_repl_client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


add_executable(003_service_repl_server
    003_service_repl_server.cpp
)

target_compile_features(003_service_repl_server
    PRIVATE
        cxx_std_17
)

target_link_libraries(003_service_repl_server
    PRIVATE
        advrf_fastdds_plugin
        advrf_interfaces_dds::fastdds
        advrf_middleware_core
        advrf_config
        yaml-cpp::yaml-cpp
)



install(
    TARGETS 003_service_repl_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
