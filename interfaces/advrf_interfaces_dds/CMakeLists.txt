cmake_minimum_required(VERSION 3.16)

project(advrf_interfaces_dds
    VERSION 0.1.0
    LANGUAGES CXX
)

option(USE_FASTDDS "Use FastDDS instead of CycloneDDS" OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(USE_FASTDDS)
    message(STATUS "[advrf_interfaces_dds] Configuring for FastDDS")
    find_package(advrf_fastdds_lib REQUIRED)
    find_package(fastcdr 2 REQUIRED)
    find_package(fastdds 3 REQUIRED)

    if(NOT ADVRF_FASTDDSGEN_EXECUTABLE OR NOT ADVRF_FASTDDSGEN_JAR)
        message(FATAL_ERROR
            "advrf_fastdds_lib did not provide the Fast DDS-Gen runtime"
        )
    endif()
else()
    message(STATUS "[advrf_interfaces_dds] Configuring for CycloneDDS")
    find_package(advrf_cyclonedds_lib REQUIRED)
    find_package(CycloneDDS REQUIRED)
    find_package(CycloneDDS-CXX REQUIRED)
endif()

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

if(USE_FASTDDS)
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
            "${DDS_GENERATED_DIR}/${IDL_RELATIVE_DIR}/${MSG_NAME}"
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
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DDS_GENERATED_DIR}"
        COMMAND ${CMAKE_COMMAND}
            -D "IDL_FILES=${FASTDDS_IDL_FILES}"
            -D "IDL_DIR=${DDS_IDL_DIR}"
            -D "OUTPUT_DIR=${DDS_GENERATED_DIR}"
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

add_library(advrf_interfaces_dds INTERFACE)
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

    if(USE_FASTDDS)
        set(ALL_GEN_SOURCES "")

        foreach(IDL_FILE IN LISTS package_idls_real)
            file(RELATIVE_PATH IDL_RELATIVE "${DDS_IDL_DIR}" "${IDL_FILE}")
            get_filename_component(IDL_RELATIVE_DIR "${IDL_RELATIVE}" DIRECTORY)
            get_filename_component(MSG_NAME "${IDL_FILE}" NAME_WE)
            set(GENERATED_BASE
                "${DDS_GENERATED_DIR}/${IDL_RELATIVE_DIR}/${MSG_NAME}"
            )
            list(APPEND ALL_GEN_SOURCES
                "${GENERATED_BASE}PubSubTypes.cxx"
                "${GENERATED_BASE}TypeObjectSupport.cxx"
            )
        endforeach()

        add_library(${package} STATIC ${ALL_GEN_SOURCES})
        add_dependencies(${package} fastdds_idl_generate)

        add_library(advrf::${package} ALIAS ${package})

        target_compile_features(${package} PUBLIC cxx_std_17)

        target_include_directories(${package}
            PUBLIC
                $<BUILD_INTERFACE:${DDS_GENERATED_DIR}>
                $<BUILD_INTERFACE:${DDS_IDL_DIR}>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

        target_compile_definitions(${package} PUBLIC USE_FASTDDS)
        target_link_libraries(${package} PUBLIC fastdds fastcdr)

    else()

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

    endif()

    install(
        TARGETS ${package}
        EXPORT advrf_interfaces_ddsTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    target_link_libraries(advrf_interfaces_dds
        INTERFACE
            advrf::${package}
    )

endforeach()

if(USE_FASTDDS)
    # Fast DDS-Gen emits calls into the generated support code of included
    # IDLs. Express those relationships so consumers of a package library get
    # all required serialization and TypeObject symbols transitively.
    target_link_libraries(std_msgs PUBLIC builtin_interfaces)
    target_link_libraries(sensor_msgs PUBLIC std_msgs)
    target_link_libraries(advrf_interfaces
        PUBLIC
            builtin_interfaces
            std_msgs
            geometry_msgs
            rcl_interfaces
    )
endif()

install(
    TARGETS advrf_interfaces_dds
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
