cmake_minimum_required(VERSION 3.1.0)
project(surskit VERSION 0.1.0)
set (CMAKE_CXX_STANDARD 20)

include_directories(
    src
)

add_subdirectory(gambit) # creats static library 'gambit'

# # Examples
# file(GLOB EXAMPLES examples/*.cc)

# foreach(EXAMPLE ${EXAMPLES})
#     get_filename_component(TEST_NAME ${EXAMPLE} NAME_WE) # get name without extension
#     add_executable(${TEST_NAME} ${EXAMPLE})
#     target_link_libraries(${TEST_NAME} gambit)
#     set_target_properties(${TEST_NAME} PROPERTIES
#                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples)
# endforeach()

# # Benchmarks
# file(GLOB BENCHMARKS benchmark/*.cc)

# foreach(BENCHMARK ${BENCHMARKS})
#     get_filename_component(TEST_NAME ${BENCHMARKS} NAME_WE) # get name without extension
#     add_executable(${TEST_NAME} ${BENCHMARKS})
#     target_link_libraries(${TEST_NAME} gambit)
#     set_target_properties(${TEST_NAME} PROPERTIES
#                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark)
# endforeach()

# # Testing
# file(GLOB TEST_FILES tests/*.cc)

# foreach(TEST_FILE ${TEST_FILES})
#     get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
#     add_executable(${TEST_NAME} ${TEST_FILE})
#     target_link_libraries(${TEST_NAME} gambit)
#     add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
#     set_target_properties(${TEST_NAME} PROPERTIES
#                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
# endforeach()
# enable_testing()

# Examples
file(GLOB EXAMPLES random-trees/*.cc)

foreach(EXAMPLE ${EXAMPLES})
    get_filename_component(TEST_NAME ${EXAMPLE} NAME_WE) # get name without extension
    add_executable(${TEST_NAME} ${EXAMPLE})
    target_link_libraries(${TEST_NAME} gambit)
    set_target_properties(${TEST_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endforeach()