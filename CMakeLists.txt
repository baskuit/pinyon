cmake_minimum_required(VERSION 3.1.0)
set (CMAKE_CXX_STANDARD 20)

project(surskit VERSION 0.1.0)

add_library(surskit INTERFACE)
target_include_directories(surskit INTERFACE src)

add_subdirectory(extern/gambit) # creats static library 'gambit'

# Testing
file(GLOB TEST_FILES tests/*.cc)

foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} gambit)
    target_link_libraries(${TEST_NAME} surskit)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_target_properties(${TEST_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
endforeach()
enable_testing()

# Python Module

add_subdirectory(extern/pybind11)
pybind11_add_module(pysurskit pybind/main.cpp)
target_link_libraries(pysurskit PRIVATE surskit)
target_link_libraries(pysurskit PRIVATE gambit)
set_target_properties(pysurskit PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_PROJECT_DIR}/pybind)