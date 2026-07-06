message(STATUS "Configuring CycloneDDS plugin")

find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)

if(NOT DDS_MSG_IDL_FILES OR DDS_MSG_IDL_FILES STREQUAL "")
    message(FATAL_ERROR "DDS_MSG_IDL_FILES missing")
endif()

message(STATUS "DDS output directory: ${CMAKE_CURRENT_BINARY_DIR}")

#
# Messages
#

idlcxx_generate(
    TARGET dds_msgs
    FILES ${DDS_MSG_IDL_FILES}
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/messages
    WARNINGS no-implicit-extensibility
)

get_target_property(_msg_sources dds_msgs INTERFACE_SOURCES)
set_target_properties(dds_msgs PROPERTIES INTERFACE_SOURCES "")
set_source_files_properties(${_msg_sources} PROPERTIES GENERATED TRUE)

#
# Services (optional)
#

if(DDS_SERVICE_IDL_FILES)

    idlcxx_generate(
        TARGET dds_services
        FILES ${DDS_SERVICE_IDL_FILES}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/services
        WARNINGS no-implicit-extensibility
    )

    get_target_property(_service_sources dds_services INTERFACE_SOURCES)
    set_target_properties(dds_services PROPERTIES INTERFACE_SOURCES "")
    set_source_files_properties(${_service_sources} PROPERTIES GENERATED TRUE)

endif()

#
# Plugin
#

add_library(ecat_plugin_cyclonedds
    src/main.cpp
    ${_msg_sources}
    ${_service_sources}
)

add_dependencies(ecat_plugin_cyclonedds
    dds_msgs_generate
)

if(TARGET dds_services_generate)
    add_dependencies(ecat_plugin_cyclonedds
        dds_services_generate
    )
endif()

target_include_directories(ecat_plugin_cyclonedds
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
        ${DDS_IDL_DIR}
)

target_link_libraries(ecat_plugin_cyclonedds
    PUBLIC
        dds_msgs
        CycloneDDS-CXX::ddscxx
)

if(TARGET dds_services)
    target_link_libraries(ecat_plugin_cyclonedds
        PUBLIC
            dds_services
    )
endif()