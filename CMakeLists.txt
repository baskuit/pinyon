cmake_minimum_required(VERSION 3.0.0)
project(surskit VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(src)
add_executable(
surskit 
src/tree/tree_linked_list.cc
src/libsurskit/math.cc
src/search/exp3.cc
src/main.cc
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)