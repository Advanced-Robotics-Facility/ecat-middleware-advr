cmake_minimum_required(VERSION 3.16)
project(advrf_middleware_docs)

option(BUILD_DOCUMENTATION "Generate ADVRF Middleware API documentation" OFF)

if(BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import breathe, sphinx, sphinx_rtd_theme"
        RESULT_VARIABLE SPHINX_DEPENDENCIES_RESULT
    )
    if(NOT SPHINX_DEPENDENCIES_RESULT EQUAL 0)
        message(FATAL_ERROR
            "Sphinx documentation dependencies are missing for ${Python3_EXECUTABLE}. "
            "Create and activate a virtual environment, then run: "
            "python -m pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/../requirements.txt"
        )
    endif()

    set(DOXYFILE ${CMAKE_CURRENT_SOURCE_DIR}/../../Doxyfile)
    set(SPHINX_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../source)
    set(SPHINX_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../build/site)

    add_custom_target(docs ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
        COMMAND ${Python3_EXECUTABLE} -m sphinx -b html ${SPHINX_SOURCE_DIR} ${SPHINX_OUTPUT_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
        COMMENT "Generating ADVRF Middleware documentation site"
        VERBATIM
    )
endif()
