cmake_minimum_required(VERSION 3.1.0)
set (CMAKE_CXX_STANDARD 20)

project(surskit VERSION 0.1.0)

add_library(surskit INTERFACE)
target_include_directories(surskit INTERFACE src)
target_include_directories(surskit INTERFACE extern/eigen)

# add_subdirectory(extern/gambit) # creats static library 'gambit'

# add_subdirectory(extern/lrslib) # creates library 'lrsnashlib'
# target_include_directories(lrsnashlib PUBLIC extern/lrslib/src)

# Testing
file(GLOB TEST_FILES tests/*.cc)

foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    # target_link_libraries(${TEST_NAME} gambit)
    target_link_libraries(${TEST_NAME} surskit)
    # target_link_libraries(${TEST_NAME} lrsnashlib)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_target_properties(${TEST_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
endforeach()
enable_testing()

# Benchmarking
file(GLOB BENCHMARK_FILES benchmark/*.cc)

foreach(FILE ${BENCHMARK_FILES})
    get_filename_component(B_NAME ${FILE} NAME_WE)
    add_executable(${B_NAME} ${FILE})
    # target_link_libraries(${B_NAME} gambit)
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