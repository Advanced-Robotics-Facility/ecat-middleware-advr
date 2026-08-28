add_executable(001_proto_pub
    001_proto_pub.cpp
)

target_compile_features(001_proto_pub 
    PRIVATE 
        cxx_std_17
)

target_link_libraries(001_proto_pub
    PRIVATE
        zenohcxx::zenohc
)

install(
    TARGETS 001_proto_pub
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---

add_executable(001_proto_sub
    001_proto_sub.cpp
)

target_compile_features(001_proto_sub 
    PRIVATE 
        cxx_std_17
)

target_link_libraries(001_proto_sub
    PRIVATE
        zenohcxx::zenohc
)

install(
    TARGETS 001_proto_sub
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)