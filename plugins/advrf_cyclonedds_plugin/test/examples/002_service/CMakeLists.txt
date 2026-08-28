add_executable(002_service_repl_client
    002_service_repl_client.cpp
)

target_compile_features(002_service_repl_client
    PRIVATE
        cxx_std_17
)

target_link_libraries(002_service_repl_client
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_interfaces_dds::cyclonedds
        advrf_middleware_core
        advrf_config
)



install(
    TARGETS 002_service_repl_client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


add_executable(002_service_repl_server
    002_service_repl_server.cpp
)

target_compile_features(002_service_repl_server
    PRIVATE
        cxx_std_17
)

target_link_libraries(002_service_repl_server
    PRIVATE
        advrf_cyclonedds_plugin
        advrf_interfaces_dds::cyclonedds
        advrf_middleware_core
        advrf_config
        yaml-cpp::yaml-cpp
)


install(
    TARGETS 002_service_repl_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
