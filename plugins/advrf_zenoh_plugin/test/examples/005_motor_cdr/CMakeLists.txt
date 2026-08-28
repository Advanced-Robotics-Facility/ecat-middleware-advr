add_executable(005_motor_cdr_viewer
    005_motor_cdr_viewer.cpp
)

target_compile_features(005_motor_cdr_viewer
    PRIVATE
        cxx_std_17
)

target_link_libraries(005_motor_cdr_viewer
    PRIVATE
        advrf_config
        advrf_middleware_core
        advrf_zenoh_plugin
        zenohcxx::zenohc
)

install(
    TARGETS 005_motor_cdr_viewer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
