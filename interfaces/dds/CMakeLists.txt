cmake_minimum_required(VERSION 3.16)

project(advrf_interfaces_dds
    VERSION 0.1.0
    LANGUAGES CXX
)

find_package(CycloneDDS REQUIRED)
find_package(CycloneDDS-CXX REQUIRED)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

get_filename_component(DDS_IDL_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/idl"
    REALPATH
)

set(DDS_GENERATED_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

set(DDS_INTERFACE_PACKAGES
    builtin_interfaces
    std_msgs
    geometry_msgs
    sensor_msgs
    rcl_interfaces
    advrf_interfaces
)

add_library(advrf_dds_interfaces INTERFACE)
add_library(advrf::dds_interfaces ALIAS advrf_dds_interfaces)

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

    set(package_idls_real "")

    foreach(idl IN LISTS package_idls)
        get_filename_component(idl_real "${idl}" REALPATH)

        file(RELATIVE_PATH rel "${DDS_IDL_DIR}" "${idl_real}")

        if(rel MATCHES "^\\.\\.")
            message(FATAL_ERROR
                "IDL file is outside DDS_IDL_DIR:\n"
                "  DDS_IDL_DIR: ${DDS_IDL_DIR}\n"
                "  IDL: ${idl_real}\n"
                "  REL: ${rel}"
            )
        endif()

        list(APPEND package_idls_real "${idl_real}")
        message(STATUS "    ${rel}")
    endforeach()

    idlcxx_generate(
        TARGET ${package}_idl
        BASE_DIR "${DDS_IDL_DIR}"
        FILES ${package_idls_real}
        OUTPUT_DIR "${DDS_GENERATED_DIR}"
        WARNINGS no-implicit-extensibility
    )

    get_target_property(package_sources ${package}_idl INTERFACE_SOURCES)

    if(NOT package_sources)
        message(FATAL_ERROR "No generated sources for package '${package}'.")
    endif()

    set_target_properties(${package}_idl
        PROPERTIES
            INTERFACE_SOURCES ""
    )

    set_source_files_properties(${package_sources}
        PROPERTIES
            GENERATED TRUE
    )

    add_library(${package} STATIC
        ${package_sources}
    )

    add_library(advrf::${package} ALIAS ${package})

    target_compile_features(${package}
        PUBLIC
            cxx_std_17
    )

    target_include_directories(${package}
        PUBLIC
            $<BUILD_INTERFACE:${DDS_GENERATED_DIR}>
            $<BUILD_INTERFACE:${DDS_IDL_DIR}>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    target_link_libraries(${package}
        PUBLIC
            CycloneDDS-CXX::ddscxx
    )

    if(TARGET ${package}_idl_generate)
        add_dependencies(${package} ${package}_idl_generate)
    endif()

    install(
        TARGETS ${package}
        EXPORT advrf_interfaces_ddsTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    target_link_libraries(advrf_dds_interfaces
        INTERFACE
            advrf::${package}
    )

endforeach()

install(
    TARGETS advrf_dds_interfaces
    EXPORT advrf_interfaces_ddsTargets
)

install(
    DIRECTORY "${DDS_GENERATED_DIR}/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

install(
    DIRECTORY "${DDS_IDL_DIR}/"
    DESTINATION "share/${PROJECT_NAME}/idl"
)

install(
    EXPORT advrf_interfaces_ddsTargets
    FILE advrf_interfaces_ddsTargets.cmake
    NAMESPACE advrf::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/advrf_interfaces_ddsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_ddsConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_ddsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_ddsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/advrf_interfaces_ddsConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)