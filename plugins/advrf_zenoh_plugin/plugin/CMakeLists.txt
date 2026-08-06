add_executable(zenoh_plugin plugin.cpp)

target_compile_features(zenoh_plugin PRIVATE cxx_std_17)

target_link_libraries(zenoh_plugin
    PRIVATE
        advrf_zenoh_plugin
        advrf_middleware_core
        yaml-cpp::yaml-cpp
)

target_compile_definitions(zenoh_plugin PRIVATE
    ROBOT_CONFIG_DIR="${advrf_middleware_core_CONFIG_DIR}/robot.yaml"
)

if(ZENOH_ROS2_SUPPORT)
    target_compile_definitions(zenoh_plugin PRIVATE
        ZENOH_ROS2_SUPPORT=1
    )
endif()

install(TARGETS zenoh_plugin RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
