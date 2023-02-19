cmake_minimum_required(VERSION 3.0.0)
project(surskit VERSION 0.1.0)

include_directories(src)
add_library(surskit src/main.cc)


add_executable(matrix_ucb src/main.cc)

# target_link_libraries()
