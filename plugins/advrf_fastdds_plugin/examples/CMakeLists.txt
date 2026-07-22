add_executable(advrf_fastdds_imu_publisher
    imu_publisher.cpp
)

target_link_libraries(advrf_fastdds_imu_publisher
    PRIVATE
        advrf_interfaces_dds
        fastdds
        fastcdr
)
