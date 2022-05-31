cmake_minimum_required(VERSION 3.0.0)
project(surskit VERSION 0.1.0)


include_directories(src)
add_library(libsurskit STATIC src/main.cc test/unit_test.cc)


add_executable(
surskit 
src/libsurskit/math.cc
src/state/toy_state.cc
src/main.cc
)

add_executable(
unit_test 
src/libsurskit/math.cc
src/state/toy_state.cc
test/unit_test.cc
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)