add_executable(fastdds_plugin
    plugin.cpp
)

target_compile_features(fastdds_plugin
    PRIVATE
        cxx_std_17
)

target_link_libraries(fastdds_plugin
    PRIVATE
        advrf_fastdds_plugin
        advrf_middleware_core
        advrf_interfaces_dds::fastdds
        advrf_config
        yaml-cpp::yaml-cpp
)

install(
    TARGETS fastdds_plugin
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)