cmake_minimum_required(VERSION 3.16)

project(shm_debug LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(shm_debug_writer
    src/writer.cpp
)

target_include_directories(shm_debug_writer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shm_debug_writer
    PRIVATE
        advrf::ecat_master_future
        advrf::protobuf_msgs
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
        advrf::ecat_master_future
        advrf::protobuf_msgs
)

install(TARGETS shm_debug_reader
    RUNTIME DESTINATION bin
)