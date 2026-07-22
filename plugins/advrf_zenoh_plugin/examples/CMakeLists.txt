add_executable(advrf_zenoh_pub
    pub.cpp
)

add_executable(advrf_zenoh_sub
    sub.cpp
)

target_compile_features(advrf_zenoh_pub PRIVATE cxx_std_17)
target_compile_features(advrf_zenoh_sub PRIVATE cxx_std_17)

target_link_libraries(advrf_zenoh_pub PRIVATE zenohcxx::zenohc)
target_link_libraries(advrf_zenoh_sub PRIVATE zenohcxx::zenohc)
