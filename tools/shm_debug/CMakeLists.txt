cmake_minimum_required(VERSION 3.16)

project(tools_shm_debug LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(advrf_interfaces_protobuf REQUIRED)
find_package(ecat_master_future REQUIRED)

add_executable(shm_debug_writer
    src/writer.cpp
)

target_include_directories(shm_debug_writer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shm_debug_writer
    PRIVATE
        advrf_interfaces_protobuf
        ecat_master_future
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
        ecat_master_future
        advrf_interfaces_protobuf
)

install(TARGETS shm_debug_reader
    RUNTIME DESTINATION bin
)