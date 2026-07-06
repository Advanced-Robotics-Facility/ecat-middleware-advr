message(STATUS "Configuring CycloneDDS plugin")

find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)

if(NOT DDS_IDL_FILES OR DDS_IDL_FILES STREQUAL "")
    message(FATAL_ERROR "DDS_IDL_FILES missing")
endif()

idlcxx_generate(
    TARGET dds_types
    FILES ${DDS_IDL_FILES}
    WARNINGS no-implicit-extensibility
)

get_target_property(_idl_sources dds_types INTERFACE_SOURCES)
set_target_properties(dds_types PROPERTIES INTERFACE_SOURCES "")
set_source_files_properties(${_idl_sources} PROPERTIES GENERATED TRUE)

add_library(ecat_plugin_cyclonedds
    src/main.cpp
    ${_idl_sources}
)

add_dependencies(ecat_plugin_cyclonedds dds_types_generate)

target_include_directories(ecat_plugin_cyclonedds PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${DDS_IDL_DIR}
)

target_link_libraries(ecat_plugin_cyclonedds PUBLIC
    dds_types
    CycloneDDS-CXX::ddscxx
)