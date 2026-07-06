option(BUILD_CYCLONEDDS "Build CycloneDDS plugin" ON)
option(BUILD_FASTDDS "Build FastDDS plugin" OFF)
option(BUILD_ZENOH "Build Zenoh plugin" OFF)
option(BUILD_ZMQ "Build ZeroMQ plugin" OFF)

if(BUILD_CYCLONEDDS)
    add_subdirectory(cyclone-dds)
endif()

if(BUILD_FASTDDS)
    add_subdirectory(fast-dds)
endif()

if(BUILD_ZENOH)
    add_subdirectory(zenoh)
endif()

if(BUILD_ZMQ)
    add_subdirectory(zmq)
endif()