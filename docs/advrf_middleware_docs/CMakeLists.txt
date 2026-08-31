cmake_minimum_required(VERSION 3.16)
project(advrf_middleware_docs)

option(BUILD_DOCUMENTATION "Generate ADVRF Middleware API documentation" OFF)

if(BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)

    set(DOXYFILE ${CMAKE_CURRENT_SOURCE_DIR}/../../Doxyfile)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
        COMMENT "Generating ADVRF Middleware documentation"
        VERBATIM
    )
endif()