cmake_minimum_required(VERSION 3.0.0)
project(surskit VERSION 0.1.0)


include_directories(src gambit/src)
add_library(libsurskit STATIC src/main.cc test/test.cc test/exp3p_thread_test.cc)


add_executable(
surskit 
src/main.cc
)

add_executable(
test 
test/exp3p_thread_test.cc
)

# add_executable(
# matrix
# gambit-maste1r/src/tools/enummixed/enummixed.cc
# # src/state_on_stack/toy_state.cc
# # src/tree/node.cc
# )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# add_library(Gambit STATIC IMPORTED)
# set_property(TARGET Gambit PROPERTY
#              IMPORTED_LOCATION "/gambit")