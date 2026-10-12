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
#find_package(advrf_middleware_core REQUIRED)
#find_package(advrf_interfaces_protobuf REQUIRED)
find_package(cppzmq REQUIRED)

message(STATUS "Configuring zmq plugin")

#add_library(ecat_plugin_zmq
#    src/main.cpp
#)

#target_compile_features(ecat_plugin_zmq PUBLIC cxx_std_17)

#target_include_directories(ecat_plugin_zmq PUBLIC
    #$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#)

#target_link_libraries(ecat_plugin_zmq
#    PUBLIC
#        advrf_zmq_lib
#        advrf_middleware_core
#        advrf_interfaces_protobuf
#        cppzmq
#)

# compile ecat_plugin_zmq
add_executable(ecat_plugin_zmq
               src/main.cpp)
               
target_include_directories(ecat_plugin_zmq
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

          
target_link_libraries(ecat_plugin_zmq
    PUBLIC
    ecat_master::ecat_core
    ecat_master::ecat_master_pb
    magic_enum::magic_enum
    ${ZeroMQ_LIBRARY}
    )

install(TARGETS ecat_plugin_zmq
        DESTINATION bin
        COMPONENT applications)

option(BUILD_TEST_EXAMPLESS "Build ZMQ examples" OFF)

if(BUILD_TEST_EXAMPLESS)
    add_subdirectory(examples)
endif()
