cmake_minimum_required(VERSION 3.16)

find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)

#
# IDL root
#

set(DDS_IDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/idl")

#
# DDS interface packages
#

set(DDS_INTERFACE_PACKAGES
    builtin_interfaces
    std_msgs
    geometry_msgs
    sensor_msgs
    rcl_interfaces
    advrf_interfaces
)

#
# Meta target
#

add_library(dds_interfaces INTERFACE)

foreach(package IN LISTS DDS_INTERFACE_PACKAGES)

    message(STATUS "[DDS] Processing package '${package}'")

    file(GLOB_RECURSE package_idls
        CONFIGURE_DEPENDS
        "${DDS_IDL_DIR}/${package}/msg/*.idl"
        "${DDS_IDL_DIR}/${package}/srv/*.idl"
    )

    if(NOT package_idls)
        message(STATUS "[DDS] ${package}: no IDL found")
        continue()
    endif()

    foreach(idl IN LISTS package_idls)
        file(RELATIVE_PATH rel "${DDS_IDL_DIR}" "${idl}")
        message(STATUS "    ${rel}")
    endforeach()

    #
    # Generate C++ sources from IDL
    #

    idlcxx_generate(
        TARGET ${package}_idl
        BASE_DIR ${DDS_IDL_DIR}
        FILES ${package_idls}
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
        WARNINGS no-implicit-extensibility
    )

    #
    # Retrieve generated sources
    #

    get_target_property(
        package_sources
        ${package}_idl
        INTERFACE_SOURCES
    )

    if(NOT package_sources)
        message(FATAL_ERROR
            "No generated sources for package '${package}'.")
    endif()

    set_target_properties(
        ${package}_idl
        PROPERTIES INTERFACE_SOURCES ""
    )

    set_source_files_properties(
        ${package_sources}
        PROPERTIES GENERATED TRUE
    )

    #
    # Build real library
    #

    add_library(${package}_dds STATIC
        ${package_sources}
    )

    target_include_directories(${package}_dds
        PUBLIC
            ${CMAKE_CURRENT_BINARY_DIR}
            ${DDS_IDL_DIR}
    )

    target_link_libraries(${package}_dds
        PUBLIC
            CycloneDDS-CXX::ddscxx
    )

    #
    # Ensure generation happens before compilation
    #

    if(TARGET ${package}_idl_generate)

        add_dependencies(
            ${package}_dds
            ${package}_idl_generate
        )

    endif()

    #
    # Meta target
    #

    target_link_libraries(dds_interfaces
        INTERFACE
            ${package}_dds
    )

endforeach()


install(
    TARGETS
        builtin_interfaces_dds
        std_msgs_dds
        geometry_msgs_dds
        sensor_msgs_dds
        rcl_interfaces_dds
        advrf_interfaces_dds
    EXPORT advrf_interfacesTargets
)