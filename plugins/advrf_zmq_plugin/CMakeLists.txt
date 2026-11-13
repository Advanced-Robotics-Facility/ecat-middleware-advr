cmake_minimum_required(VERSION 3.16)

project(advrf_zmq_plugin
    VERSION 0.1.0
    LANGUAGES C CXX
)

#if(NOT BUILD_MIDDLEWARE_ZMQ)
    #message(STATUS "ZMQ build disabled")
    #return()
#endif()

## use the hint from about to find the location of libzmq
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

find_package(ecat_master REQUIRED COMPONENTS ecat_core)
find_package(advrf_zmq_lib REQUIRED)
find_package(advrf_middleware_core REQUIRED)
find_package(cppzmq REQUIRED)

message(STATUS "Configuring zmq plugin")

# compile zmq_plugin
add_executable(zmq_plugin
               plugin/plugin.cpp)
               
target_include_directories(zmq_plugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

          
target_link_libraries(zmq_plugin
    PUBLIC
    ecat_master::ecat_core
    ecat_master::ecat_master_pb
    magic_enum::magic_enum
    advrf_middleware_core
    ${ZeroMQ_LIBRARY}
    )

install(TARGETS zmq_plugin
        DESTINATION bin
        COMPONENT applications)