cmake_minimum_required(VERSION 3.0)

project(resilient CXX)
add_compile_options(-std=c++14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0") # TODO remove
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

# set(HEADER_FILES "...")
add_library(resilient INTERFACE)
# target_sources(resilient INTERFACE ${HEADER_FILES})
target_include_directories(resilient INTERFACE "include/" "include/resilient/")

enable_testing()

add_subdirectory("/usr/src/gmock" "${CMAKE_BINARY_DIR}/vendor/gmock") # TODO make the path an option

file(GLOB test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*.t.cpp)
add_executable(resilient_test ${test_files})
target_link_libraries(resilient_test resilient gtest gtest_main gmock)
add_test(NAME mytest COMMAND resilient_test)