add_executable(repl_client
    repl_client.cpp
)

target_compile_features(repl_client
    PRIVATE
        cxx_std_17
)

target_link_libraries(repl_client
    PRIVATE
        advrf_cyclonedds_plugin
)

add_test(
    NAME repl_client
    COMMAND repl_client
)

install(
    TARGETS repl_client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)