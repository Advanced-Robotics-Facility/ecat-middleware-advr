cmake_minimum_required(VERSION 3.16)

project(advrf_interfaces_dds
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(BUILD_FASTDDS "Build FastDDS backend" OFF)
option(BUILD_CYCLONEDDS "Build CycloneDDS backend" OFF)
option(BUILD_ZENOH "Build Zenoh backend" OFF)

if(NOT BUILD_FASTDDS AND NOT BUILD_CYCLONEDDS AND NOT BUILD_ZENOH)
    message(FATAL_ERROR
        "At least one DDS (or Zenoh) backend must be enabled."
    )
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

#
# FastDDS
#

if(BUILD_FASTDDS)

    message(STATUS "[advrf_interfaces_dds] Configuring FastDDS")

    find_package(advrf_fastdds_lib REQUIRED)
    find_package(fastcdr REQUIRED)
    find_package(fastdds REQUIRED)

    if(NOT ADVRF_FASTDDSGEN_EXECUTABLE
       OR NOT ADVRF_FASTDDSGEN_JAR)

        message(FATAL_ERROR
            "advrf_fastdds_lib did not provide FastDDSGen."
        )

    endif()

endif()

#
# Zenoh
#

if(BUILD_ZENOH)
    message(STATUS "[advrf_interfaces_dds] Configuring CDR-only Zenoh types")

    find_package(advrf_zenoh_lib REQUIRED)
    find_package(fastcdr REQUIRED)

    if(NOT ADVRF_FASTDDSGEN_EXECUTABLE
       OR NOT ADVRF_FASTDDSGEN_JAR)
        message(FATAL_ERROR
            "advrf_zenoh_lib did not provide FastDDSGen."
        )
    endif()
endif()

#
# CycloneDDS
#

if(BUILD_CYCLONEDDS)

    message(STATUS "[advrf_interfaces_dds] Configuring CycloneDDS")

    find_package(advrf_cyclonedds_lib REQUIRED)
    find_package(CycloneDDS REQUIRED)
    find_package(CycloneDDS-CXX REQUIRED)

endif()

get_filename_component(DDS_IDL_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/idl"
    REALPATH
)

set(FASTDDS_GENERATED_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated/fastdds"
)

set(CYCLONEDDS_GENERATED_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated/cyclonedds"
)

set(DDS_INTERFACE_PACKAGES
    builtin_interfaces
    std_msgs
    geometry_msgs
    sensor_msgs
    rcl_interfaces
    advrf_interfaces
    rmw_dds_common
)

if(BUILD_FASTDDS OR BUILD_ZENOH)
    set(FASTDDS_GENERATED_OUTPUTS "")

    file(GLOB_RECURSE FASTDDS_IDL_FILES
        CONFIGURE_DEPENDS
        "${DDS_IDL_DIR}/*/msg/*.idl"
        "${DDS_IDL_DIR}/*/srv/*.idl"
    )

    foreach(IDL_FILE IN LISTS FASTDDS_IDL_FILES)
        file(RELATIVE_PATH IDL_RELATIVE "${DDS_IDL_DIR}" "${IDL_FILE}")
        get_filename_component(IDL_RELATIVE_DIR "${IDL_RELATIVE}" DIRECTORY)
        get_filename_component(MSG_NAME "${IDL_FILE}" NAME_WE)
        set(GENERATED_BASE
            "${FASTDDS_GENERATED_DIR}/${IDL_RELATIVE_DIR}/${MSG_NAME}"
        )

        list(APPEND FASTDDS_GENERATED_OUTPUTS
            "${GENERATED_BASE}.hpp"
            "${GENERATED_BASE}CdrAux.hpp"
            "${GENERATED_BASE}CdrAux.ipp"
            "${GENERATED_BASE}PubSubTypes.cxx"
            "${GENERATED_BASE}PubSubTypes.hpp"
            "${GENERATED_BASE}TypeObjectSupport.cxx"
            "${GENERATED_BASE}TypeObjectSupport.hpp"
        )
    endforeach()

    add_custom_command(
        OUTPUT ${FASTDDS_GENERATED_OUTPUTS}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${FASTDDS_GENERATED_DIR}"
        COMMAND ${CMAKE_COMMAND}
            -D "IDL_FILES=${FASTDDS_IDL_FILES}"
            -D "IDL_DIR=${DDS_IDL_DIR}"
            -D "OUTPUT_DIR=${FASTDDS_GENERATED_DIR}"
            -D "FASTDDSGEN_EXECUTABLE=${ADVRF_FASTDDSGEN_EXECUTABLE}"
            -D "FASTDDSGEN_JAR=${ADVRF_FASTDDSGEN_JAR}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/run_fastddsgen.cmake"
        DEPENDS
            ${FASTDDS_IDL_FILES}
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/run_fastddsgen.cmake"
            "${ADVRF_FASTDDSGEN_JAR}"
        COMMENT "FastDDSGen: generating IDL sources"
        VERBATIM
    )

    set_source_files_properties(${FASTDDS_GENERATED_OUTPUTS}
        PROPERTIES GENERATED TRUE
    )
    add_custom_target(fastdds_idl_generate
        DEPENDS ${FASTDDS_GENERATED_OUTPUTS}
    )

endif()

if(BUILD_FASTDDS)
    add_library(advrf_interfaces_dds_fastdds INTERFACE)
    add_library(advrf_interfaces_dds::fastdds ALIAS advrf_interfaces_dds_fastdds)

    set_target_properties(advrf_interfaces_dds_fastdds PROPERTIES
        EXPORT_NAME fastdds
    )

    install(
        TARGETS advrf_interfaces_dds_fastdds
        EXPORT advrf_interfaces_ddsTargets
    )
endif()

if(BUILD_ZENOH)
    add_library(advrf_interfaces_dds_cdr INTERFACE)
    add_library(advrf_interfaces_dds::cdr ALIAS advrf_interfaces_dds_cdr)

    set_target_properties(advrf_interfaces_dds_cdr PROPERTIES
        EXPORT_NAME cdr
    )

    install(
        TARGETS advrf_interfaces_dds_cdr
        EXPORT advrf_interfaces_ddsTargets
    )
endif()

if(BUILD_CYCLONEDDS)
    add_library(advrf_interfaces_dds_cyclonedds INTERFACE)
    add_library(advrf_interfaces_dds::cyclonedds ALIAS advrf_interfaces_dds_cyclonedds)

    set_target_properties(advrf_interfaces_dds_cyclonedds PROPERTIES
        EXPORT_NAME cyclonedds
    )

    install(
        TARGETS advrf_interfaces_dds_cyclonedds
        EXPORT advrf_interfaces_ddsTargets
    )
endif()

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

    # --- FastDDS ---
    if(BUILD_FASTDDS)
        set(target_fastdds "${package}_fastdds")
        set(ALL_GEN_SOURCES "")

        foreach(IDL_FILE IN LISTS package_idls_real)
            file(RELATIVE_PATH IDL_RELATIVE "${DDS_IDL_DIR}" "${IDL_FILE}")
            get_filename_component(IDL_RELATIVE_DIR "${IDL_RELATIVE}" DIRECTORY)
            get_filename_component(MSG_NAME "${IDL_FILE}" NAME_WE)
            set(GENERATED_BASE
                "${FASTDDS_GENERATED_DIR}/${IDL_RELATIVE_DIR}/${MSG_NAME}"
            )
            list(APPEND ALL_GEN_SOURCES
                "${GENERATED_BASE}PubSubTypes.cxx"
                "${GENERATED_BASE}TypeObjectSupport.cxx"
            )
        endforeach()

        add_library(${target_fastdds} STATIC ${ALL_GEN_SOURCES})
        add_dependencies(${target_fastdds} fastdds_idl_generate)

        target_compile_features(${target_fastdds} PUBLIC cxx_std_17)

        target_include_directories(${target_fastdds}
            PUBLIC
                $<BUILD_INTERFACE:${FASTDDS_GENERATED_DIR}>
                $<BUILD_INTERFACE:${DDS_IDL_DIR}>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/fastdds>
        )

        target_compile_definitions(${target_fastdds} PUBLIC USE_FASTDDS)
        target_link_libraries(${target_fastdds} PUBLIC fastdds fastcdr)

        install(
            TARGETS ${target_fastdds}
            EXPORT advrf_interfaces_ddsTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fastdds
        )

        target_link_libraries(advrf_interfaces_dds_fastdds
            INTERFACE
                ${target_fastdds}
        )
    endif()

    # --- Zenoh --- 
    if(BUILD_ZENOH)
        set(target_cdr "${package}_cdr")
        set(cdr_wrapper_sources "")

        foreach(IDL_FILE IN LISTS package_idls_real)
            file(RELATIVE_PATH IDL_RELATIVE "${DDS_IDL_DIR}" "${IDL_FILE}")
            get_filename_component(IDL_RELATIVE_DIR "${IDL_RELATIVE}" DIRECTORY)
            get_filename_component(MSG_NAME "${IDL_FILE}" NAME_WE)

            set(wrapper_dir
                "${CMAKE_CURRENT_BINARY_DIR}/generated/cdr_sources/${IDL_RELATIVE_DIR}"
            )
            set(wrapper_source "${wrapper_dir}/${MSG_NAME}CdrAux.cxx")
            file(MAKE_DIRECTORY "${wrapper_dir}")
            file(GENERATE
                OUTPUT "${wrapper_source}"
                CONTENT "#include \"${IDL_RELATIVE_DIR}/${MSG_NAME}CdrAux.ipp\"\n"
            )
            list(APPEND cdr_wrapper_sources "${wrapper_source}")
        endforeach()

        add_library(${target_cdr} STATIC ${cdr_wrapper_sources})
        add_dependencies(${target_cdr} fastdds_idl_generate)

        target_compile_features(${target_cdr} PUBLIC cxx_std_17)
        target_include_directories(${target_cdr}
            PUBLIC
                $<BUILD_INTERFACE:${FASTDDS_GENERATED_DIR}>
                $<BUILD_INTERFACE:${DDS_IDL_DIR}>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cdr>
        )
        target_link_libraries(${target_cdr} PUBLIC fastcdr)

        install(
            TARGETS ${target_cdr}
            EXPORT advrf_interfaces_ddsTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cdr
        )

        target_link_libraries(advrf_interfaces_dds_cdr INTERFACE ${target_cdr})
    endif()

    # --- CycloneDDS ---
    if(BUILD_CYCLONEDDS)
        set(target_cyclonedds "${package}_cyclonedds")

        idlcxx_generate(
            TARGET ${target_cyclonedds}_idl
            BASE_DIR "${DDS_IDL_DIR}"
            FILES ${package_idls_real}
            OUTPUT_DIR "${CYCLONEDDS_GENERATED_DIR}"
            WARNINGS no-implicit-extensibility
        )

        get_target_property(package_sources ${target_cyclonedds}_idl INTERFACE_SOURCES)

        if(NOT package_sources)
            message(FATAL_ERROR "No generated sources for package '${package}' (CycloneDDS).")
        endif()

        set_target_properties(${target_cyclonedds}_idl
            PROPERTIES
                INTERFACE_SOURCES ""
        )

        set_source_files_properties(${package_sources}
            PROPERTIES
                GENERATED TRUE
        )

        add_library(${target_cyclonedds} STATIC
            ${package_sources}
        )

        target_compile_features(${target_cyclonedds}
            PUBLIC
                cxx_std_17
        )

        target_include_directories(${target_cyclonedds}
            PUBLIC
                $<BUILD_INTERFACE:${CYCLONEDDS_GENERATED_DIR}>
                $<BUILD_INTERFACE:${DDS_IDL_DIR}>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cyclonedds>
        )

        target_link_libraries(${target_cyclonedds}
            PUBLIC
                CycloneDDS-CXX::ddscxx
        )

        if(TARGET ${target_cyclonedds}_idl_generate)
            add_dependencies(${target_cyclonedds} ${target_cyclonedds}_idl_generate)
        endif()

        install(
            TARGETS ${target_cyclonedds}
            EXPORT advrf_interfaces_ddsTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cyclonedds
        )

        target_link_libraries(advrf_interfaces_dds_cyclonedds
            INTERFACE
                ${target_cyclonedds}
        )
    endif()

endforeach()

#
# Inter-packages dependencies FastDDS
#
if(BUILD_FASTDDS)
    # Fast DDS-Gen emits calls into the generated support code of included
    # IDLs. Express those relationships so consumers of a package library get
    # all required serialization and TypeObject symbols transitively.
    target_link_libraries(std_msgs_fastdds PUBLIC builtin_interfaces_fastdds)
    target_link_libraries(sensor_msgs_fastdds PUBLIC std_msgs_fastdds)
    target_link_libraries(advrf_interfaces_fastdds
        PUBLIC
            builtin_interfaces_fastdds
            std_msgs_fastdds
            geometry_msgs_fastdds
            rcl_interfaces_fastdds
    )

    install(
        DIRECTORY "${FASTDDS_GENERATED_DIR}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fastdds"
        FILES_MATCHING
            PATTERN "*.hpp"
            PATTERN "*.h"
            PATTERN "*.ipp"
    )
endif()

if(BUILD_ZENOH)
    target_link_libraries(std_msgs_cdr PUBLIC builtin_interfaces_cdr)
    target_link_libraries(sensor_msgs_cdr
        PUBLIC
            std_msgs_cdr
            geometry_msgs_cdr
    )
    target_link_libraries(advrf_interfaces_cdr
        PUBLIC
            builtin_interfaces_cdr
            std_msgs_cdr
            geometry_msgs_cdr
            rcl_interfaces_cdr
    )

    install(
        DIRECTORY "${FASTDDS_GENERATED_DIR}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cdr"
        FILES_MATCHING
            PATTERN "*.hpp"
            PATTERN "*.h"
            PATTERN "*.ipp"
    )
endif()

#
# Installation CycloneDDS
#
if(BUILD_CYCLONEDDS)
    install(
        DIRECTORY "${CYCLONEDDS_GENERATED_DIR}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cyclonedds"
        FILES_MATCHING
            PATTERN "*.hpp"
            PATTERN "*.h"
    )
endif()

install(
    DIRECTORY "${DDS_IDL_DIR}/"
    DESTINATION "share/${PROJECT_NAME}/idl"
)

# --- CORRECTION ICI ---
install(
    EXPORT advrf_interfaces_ddsTargets
    FILE advrf_interfaces_ddsTargets.cmake
    NAMESPACE advrf_interfaces_dds::
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