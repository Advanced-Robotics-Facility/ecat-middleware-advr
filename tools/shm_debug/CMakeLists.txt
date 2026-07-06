cmake_minimum_required(VERSION 3.16)

project(shm_debug LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ecat-master-future CONFIG REQUIRED)
find_package(protobuf_msgs CONFIG REQUIRED)

add_executable(shm_debug_writer
    src/writer.cpp
)

target_include_directories(shm_debug_writer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shm_debug_writer
    PRIVATE
        ecat-master-future::ecat-master-future
        protobuf_msgs::protobuf_msgs
)

install(TARGETS shm_debug_writer
    RUNTIME DESTINATION bin
)


add_executable(shm_debug_reader
    src/reader.cpp
)

target_include_directories(shm_debug_reader
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shm_debug_reader
    PRIVATE
        ecat-master-future::ecat-master-future
        protobuf_msgs::protobuf_msgs
)

install(TARGETS shm_debug_reader
    RUNTIME DESTINATION bin
)