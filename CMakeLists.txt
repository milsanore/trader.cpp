cmake_minimum_required(VERSION 3.16)
project(tradercpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# Define the source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable
add_executable(tradercpp ${SOURCES})

############################################

# Conan-generated toolchain setup
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL RESULT_VARIABLE _found_toolchain)
if(_found_toolchain)
    message(STATUS "Conan toolchain included")
endif()

# Find packages from Conan
find_package(concurrentqueue REQUIRED)
find_package(ftxui REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(quickfix REQUIRED CONFIG)
find_package(libsodium REQUIRED)
find_package(spdlog REQUIRED)

# Link them to the executable
target_link_libraries(tradercpp PRIVATE
    concurrentqueue::concurrentqueue
    ftxui::ftxui
    OpenSSL::SSL
    OpenSSL::Crypto
    quickfix::quickfix
    libsodium::libsodium
    spdlog::spdlog
)
