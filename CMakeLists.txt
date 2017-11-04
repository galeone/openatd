cmake_minimum_required (VERSION 3.1)
project (openatd)

#
# BUILD SETTINGS
#

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
# Create compile_commands.json in build dir while compiling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compilation flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

#
# BUILD DEPENDENCIES
#
# Build and setup the correct cmake variables for third-party libraries
#

# Build OpenAT
set(OPENAT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/libs/openat")
set(OPENAT_INCLUDE_DIR "${OPENAT_SOURCE_DIR}/include")
add_subdirectory(${OPENAT_SOURCE_DIR})


# Spdlog is a header-only library
set(SPDLOG "${PROJECT_SOURCE_DIR}/libs/spdlog")
set(SPDLOG_INCLUDE_DIR "${SPDLOG}/include")

set(SQLITECPP "${PROJECT_SOURCE_DIR}/libs/SQLiteCpp")
set(SQLITECPP_INCLUDE_DIR "${SQLITECPP}/include")
# Compile and define SQLiteCpp
add_subdirectory(${SQLITECPP})


set(GTEST "${PROJECT_SOURCE_DIR}/libs/googletest")
set(GTEST_INCLUDE_DIR "${GTEST}/include")
if(NOT TARGET gtest)
    # Build googletest
    add_subdirectory(${GTEST})
endif()

#
# Build project
#

set(OPENATD_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
add_subdirectory(src)

#
# Build tests
#
#enable_testing()
#add_subdirectory(tests)
#add_test (NAME openatd_test COMMAND runUnitTests)

# copy compile commands from build dir to project dir once compiled
ADD_CUSTOM_TARGET(do_always ALL COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${PROJECT_SOURCE_DIR}/compile_commands.json)

