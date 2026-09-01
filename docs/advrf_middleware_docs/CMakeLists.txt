cmake_minimum_required(VERSION 3.16)
project(advrf_middleware_docs)

option(BUILD_DOCUMENTATION "Generate ADVRF Middleware API documentation" OFF)

if(BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)
    find_program(SPHINX_BUILD sphinx-build REQUIRED)

    set(DOXYFILE ${CMAKE_CURRENT_SOURCE_DIR}/../../Doxyfile)
    set(SPHINX_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../source)
    set(SPHINX_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../build/site)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
        COMMAND ${SPHINX_BUILD} -b html ${SPHINX_SOURCE_DIR} ${SPHINX_OUTPUT_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
        COMMENT "Generating ADVRF Middleware documentation site"
        VERBATIM
    )
endif()
