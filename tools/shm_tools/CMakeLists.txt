cmake_minimum_required(VERSION 3.16)

project(shm_tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Curses REQUIRED)
find_package(advrf_middleware_core REQUIRED)
find_package(advrf_interfaces_protobuf REQUIRED)

#
# Bridge Inspector
#

add_executable(bridge_inspector
    src/main.cpp
)

target_include_directories(bridge_inspector
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CURSES_INCLUDE_DIRS}
)

target_link_libraries(bridge_inspector
    PRIVATE
        advrf_interfaces_protobuf
        advrf_middleware_core
         ${CURSES_LIBRARIES}
)

install(TARGETS bridge_inspector
    RUNTIME DESTINATION bin
)