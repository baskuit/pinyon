cmake_minimum_required(VERSION 3.1.0)
set (CMAKE_CXX_STANDARD 23)

project(surskit VERSION 0.1.0)

option(ENABLE_TORCH "Enable Libtorch" OFF)
option(ENABLE_CONCEPTS "Enable Concepts" ON)

add_library(surskit INTERFACE)
target_include_directories(surskit INTERFACE src)

if(ENABLE_TORCH)
    set(CMAKE_PREFIX_PATH "/home/user/Desktop/libtorch/")
    set(CMAKE_CUDA_ARCHITECTURES 61)
    # change this depending on your CUDA version
    set(CMAKE_CUDA_COMPILER /usr/local/cuda-12.1/bin/nvcc)
    # change this depending on installation path
    find_package(Torch REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
    target_link_libraries(surskit INTERFACE "${TORCH_LIBRARIES}")
endif()

if (ENABLE_CONCEPTS)
    target_compile_definitions(surskit INTERFACE ENABLE_CONCEPTS)
endif()

add_subdirectory(extern/lrslib)

target_link_libraries(surskit INTERFACE lrsnashlib)

# Testing
file(GLOB_RECURSE TEST_FILES tests/*.cc)

foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    string(REPLACE "/" "_" TEST_TARGET_NAME ${TEST_NAME})

    add_executable(${TEST_TARGET_NAME} ${TEST_FILE})

    target_link_libraries(${TEST_TARGET_NAME} surskit)
    if (ENABLE_TORCH)
        target_link_libraries(${TEST_NAME} "${TORCH_LIBRARIES}")
    endif()

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
