cmake_minimum_required(VERSION 3.16)
project(tradercpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)


# === App Source Files ==========================

# Collect source files
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")
# Separate main.cpp from the rest of the sources
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*main\\.cpp$")
set(SOURCES ${ALL_SOURCES})


# === Create core logic library =================

# This allows unit tests to run
# Link dependencies to the *library* (so both exe and tests get them)
add_library(traderlib STATIC ${SOURCES})


# === Conan Setup ===============================

# Conan-generated toolchain setup
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL RESULT_VARIABLE _found_toolchain)
if(_found_toolchain)
    message(STATUS "Conan toolchain included")
endif()

find_package(absl REQUIRED)
find_package(Boost REQUIRED)
find_package(concurrentqueue REQUIRED)
find_package(efsw REQUIRED)
find_package(ftxui REQUIRED)
find_package(gperftools REQUIRED)
find_package(Microsoft.GSL REQUIRED)
find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(quickfix REQUIRED CONFIG)
find_package(libsodium REQUIRED)
find_package(spdlog REQUIRED)

target_link_libraries(traderlib PUBLIC
    abseil::abseil
    boost::boost
    concurrentqueue::concurrentqueue
    efsw::efsw
    ftxui::ftxui
    gperftools::gperftools
    Microsoft.GSL::GSL
    OpenSSL::SSL
    OpenSSL::Crypto
    quickfix::quickfix
    libsodium::libsodium
    spdlog::spdlog
)


# === Create main executable ====================

add_executable(tradercpp "src/main.cpp")
target_link_libraries(tradercpp PRIVATE traderlib)


# === Enable `ctest` ============================

enable_testing()
add_subdirectory(tests)
