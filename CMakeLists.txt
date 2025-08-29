cmake_minimum_required(VERSION 3.16)
project(tradercpp)

# Conan-generated toolchain setup
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL RESULT_VARIABLE _found_toolchain)
if(_found_toolchain)
    message(STATUS "Conan toolchain included")
endif()

set(CMAKE_CXX_STANDARD 17)

# Define the source files
set(SOURCES src/main.cpp)

# Create the executable
add_executable(tradercpp ${SOURCES})
