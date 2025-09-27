cmake_minimum_required(VERSION 3.16)
project(tradercpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

############################################

# Conan-generated toolchain setup
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL RESULT_VARIABLE _found_toolchain)
if(_found_toolchain)
    message(STATUS "Conan toolchain included")
endif()

# Find packages from Conan
find_package(concurrentqueue REQUIRED)
find_package(ftxui REQUIRED)
find_package(Microsoft.GSL REQUIRED)
find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(quickfix REQUIRED CONFIG)
find_package(libsodium REQUIRED)
find_package(spdlog REQUIRED)

############################################

# Collect source files
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")
# Separate main.cpp from the rest of the sources
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*main\\.cpp$")
set(SOURCES ${ALL_SOURCES})

# Create core logic library
add_library(traderlib STATIC ${SOURCES})

# Add coverage flags to the traderlib target
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Enabling coverage flags")
    target_compile_options(traderlib PRIVATE -O0 -g --coverage)
    target_link_libraries(traderlib PRIVATE --coverage)
endif()

# Link dependencies to the library (so both exe and tests get them)
target_link_libraries(traderlib PUBLIC
    concurrentqueue::concurrentqueue
    ftxui::ftxui
    Microsoft.GSL::GSL
    OpenSSL::SSL
    OpenSSL::Crypto
    quickfix::quickfix
    libsodium::libsodium
    spdlog::spdlog
)

############################################
# Create main executable
add_executable(tradercpp "src/main.cpp")
target_link_libraries(tradercpp PRIVATE traderlib)

############################################

# Run clang-format on all source files before building
add_custom_command(
    TARGET tradercpp
    PRE_BUILD
    COMMAND clang-format -i ${CMAKE_SOURCE_DIR}/src/**/*.cpp ${CMAKE_SOURCE_DIR}/src/**/*.h
         && clang-format -i ${CMAKE_SOURCE_DIR}/tests/**/*.cpp ${CMAKE_SOURCE_DIR}/tests/**/*.h
    COMMENT "Running clang-format on all source and header files"
)

# set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-config-file=.clang-tidy;-checks=*")

############################################
# Enable `ctest`
enable_testing()
add_subdirectory(tests)
