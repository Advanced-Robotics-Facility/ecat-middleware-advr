find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)

#
# Plugin
#

add_library(ecat_plugin_cyclonedds
    src/main.cpp
    src/parameter.cpp
    src/service.cpp
)

target_include_directories(ecat_plugin_cyclonedds
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ecat_plugin_cyclonedds
    PUBLIC
        mw_core
        advrf_interfaces_dds
        rcl_interfaces_dds
        geometry_msgs_dds
        sensor_msgs_dds
        std_msgs_dds
        builtin_interfaces_dds
        CycloneDDS-CXX::ddscxx
        
)

enable_testing()
add_subdirectory(test)