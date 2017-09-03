cmake_minimum_required(VERSION 3.0)

project(resilient CXX)
add_compile_options(-std=c++14)

# set(HEADER_FILES "...")
add_library(resilient INTERFACE)
# target_sources(resilient INTERFACE ${HEADER_FILES})
target_include_directories(resilient INTERFACE "include/" "include/resilient/")

enable_testing()

add_subdirectory("/usr/src/gtest" "${CMAKE_BINARY_DIR}/gtest") # TODO make the path an option

add_executable(resilient_test "test/resilient.t.cpp")
target_link_libraries(resilient_test resilient gtest gtest_main)
add_test(NAME mytest COMMAND resilient_test)