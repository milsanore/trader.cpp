cmake_minimum_required(VERSION 3.16)
project(tradercpp)

set(CMAKE_CXX_STANDARD 17)

# Define the source files
set(SOURCES src/main.cpp)

# Create the executable
add_executable(tradercpp ${SOURCES})
