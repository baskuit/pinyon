cmake_minimum_required(VERSION 3.1.0)
set (CMAKE_CXX_STANDARD 23)

project(surskit VERSION 0.1.0)

add_library(surskit INTERFACE)
target_include_directories(surskit INTERFACE src)
# target_include_directories(surskit INTERFACE extern/eigen)

add_subdirectory(extern/lrslib)

target_link_libraries(surskit INTERFACE lrsnashlib)

# Testing
file(GLOB_RECURSE TEST_FILES tests/*.cc)

foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    string(REPLACE "/" "_" TEST_TARGET_NAME ${TEST_NAME})

    add_executable(${TEST_TARGET_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_TARGET_NAME} surskit)
    target_compile_definitions(${TEST_TARGET_NAME} PRIVATE fconcepts-diagnostics-depth=50)

    add_test(NAME ${TEST_TARGET_NAME} COMMAND ${TEST_TARGET_NAME})
    set_target_properties(${TEST_TARGET_NAME} PROPERTIES
                          RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
endforeach()
enable_testing()

# Benchmarking
file(GLOB BENCHMARK_FILES benchmark/*.cc)

foreach(FILE ${BENCHMARK_FILES})
    get_filename_component(B_NAME ${FILE} NAME_WE)
    add_executable(${B_NAME} ${FILE})
    target_link_libraries(${B_NAME} surskit)
    set_target_properties(${B_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark)
endforeach()

# Python Module

# add_subdirectory(extern/pybind11)
# pybind11_add_module(pysurskit pybind/main.cpp)
# target_link_libraries(pysurskit PRIVATE surskit)
# # target_link_libraries(pysurskit PRIVATE gambit)
# set_target_properties(pysurskit PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/pybind)