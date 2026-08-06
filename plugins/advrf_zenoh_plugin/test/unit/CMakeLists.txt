add_executable(zenoh_protobuf_serialization
    protobuf_serialization.cpp
)

target_compile_features(zenoh_protobuf_serialization PRIVATE cxx_std_17)
target_link_libraries(zenoh_protobuf_serialization
    PRIVATE
        advrf_zenoh_plugin
)

add_test(
    NAME zenoh_protobuf_serialization
    COMMAND zenoh_protobuf_serialization
)
