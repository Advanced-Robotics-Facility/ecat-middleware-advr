cmake_minimum_required(VERSION 3.16)

find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)

add_library(ecat_plugin_cyclonedds
    src/main.cpp
    src/parameter_make_get.cpp
    src/service.cpp
    src/parameter_client.cpp
    src/parameter_server.cpp
)

target_include_directories(ecat_plugin_cyclonedds
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(ecat_plugin_cyclonedds
    PUBLIC
        advrf_interfaces_dds
        rcl_interfaces_dds
        geometry_msgs_dds
        sensor_msgs_dds
        std_msgs_dds
        builtin_interfaces_dds
        CycloneDDS-CXX::ddscxx
        
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/ecat_plugin_cycloneddsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ecat_plugin_cycloneddsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ecat_plugin_cyclonedds
)

install(
    TARGETS ecat_plugin_cyclonedds
    EXPORT ecat_plugin_cycloneddsTargets

    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}

    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT ecat_plugin_cycloneddsTargets
    FILE ecat_plugin_cycloneddsTargets.cmake
    NAMESPACE ecat::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ecat_plugin_cyclonedds
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ecat_plugin_cycloneddsConfig.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/ecat_plugin_cyclonedds
)

enable_testing()
add_subdirectory(test)


## TEST PUB EXE

add_executable(publisher src/publisher.cpp)

target_include_directories(publisher PRIVATE
    ${CMAKE_BINARY_DIR}/include
)

target_link_libraries(publisher PRIVATE
    ecat-master-future     
    protobuf_msgs 
    mw_core
    CycloneDDS-CXX::ddscxx    
    advrf_interfaces_dds
    rcl_interfaces_dds
    geometry_msgs_dds
    sensor_msgs_dds
    std_msgs_dds
    builtin_interfaces_dds
)

## TEST PUB EXE