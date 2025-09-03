cmake_minimum_required(VERSION 3.16)
project(tradercpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# Define the source files
file(GLOB SOURCES src/*.cpp)

# Create the executable
add_executable(tradercpp ${SOURCES})

############################################

# Conan-generated toolchain setup
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL RESULT_VARIABLE _found_toolchain)
if(_found_toolchain)
    message(STATUS "Conan toolchain included")
endif()

# Find packages from Conan
find_package(libsodium REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(quickfix REQUIRED CONFIG)

# Link them to the executable
target_link_libraries(tradercpp PRIVATE
    libsodium::libsodium
    OpenSSL::SSL
    OpenSSL::Crypto
    quickfix::quickfix
)
