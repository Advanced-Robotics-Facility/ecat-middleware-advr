add_executable(004_service_repl_client
    004_service_repl_client.cpp
)

target_compile_features(004_service_repl_client
    PRIVATE
        cxx_std_17
)

target_link_libraries(004_service_repl_client
    PRIVATE
        advrf_cyclonedds_plugin
)



install(
    TARGETS 004_service_repl_client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


add_executable(004_service_repl_server
    004_service_repl_server.cpp
)

target_compile_features(004_service_repl_server
    PRIVATE
        cxx_std_17
)

target_link_libraries(004_service_repl_server
    PRIVATE
        advrf_cyclonedds_plugin
        yaml-cpp::yaml-cpp
)



target_compile_definitions(004_service_repl_server PRIVATE
    ROBOT_CONFIG_DIR="${ecat_master_future_CONFIG_DIR}/robot.yaml"
)


install(
    TARGETS 004_service_repl_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)