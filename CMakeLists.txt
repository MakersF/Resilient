cmake_minimum_required(VERSION 3.0)

project(resilient CXX)
OPTION(CPP17 "Compile with c++17" OFF)
IF(CPP17)
    add_compile_options(-std=gnu++1z)
ELSE()
    add_compile_options(-std=c++14)
ENDIF()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -Wextra -Wpedantic -ftemplate-backtrace-limit=0") # TODO remove -O0
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ftemplate-backtrace-limit=0")

OPTION(FORCE_BOOST "Force boost" OFF)
IF(FORCE_BOOST)
    ADD_DEFINITIONS(-DFORCE_BOOST=1)
ENDIF()

# set(HEADER_FILES "...")
add_library(resilient INTERFACE)
# target_sources(resilient INTERFACE ${HEADER_FILES})
target_include_directories(resilient INTERFACE "include/")

enable_testing()

add_subdirectory("/usr/src/gmock" "${CMAKE_BINARY_DIR}/vendor/gmock") # TODO make the path an option

file(GLOB test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*/*.t.cpp)
add_executable(resilient_test ${test_files})
target_include_directories(resilient_test PUBLIC "." "test/")
target_link_libraries(resilient_test resilient gtest gtest_main gmock)
add_test(NAME mytest COMMAND resilient_test)