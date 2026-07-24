add_executable(protobuf_compliant_pub
    001_pub.cpp
)

add_executable(protobuf_compliant_sub
    001_sub.cpp
)

target_compile_features(protobuf_compliant_pub PRIVATE cxx_std_17)
target_compile_features(protobuf_compliant_sub PRIVATE cxx_std_17)

target_link_libraries(protobuf_compliant_pub
    PRIVATE
        zenohcxx::zenohc
        ecat_master_future
)

target_link_libraries(protobuf_compliant_sub
    PRIVATE
        zenohcxx::zenohc
        ecat_master_future
)