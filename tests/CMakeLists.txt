# file(GLOB_RECURSE TEST_SOURCES
#     "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
# )

file(GLOB_RECURSE TEST_SOURCES
    binance/*_test.cpp
    core/*_test.cpp
    ui/*_test.cpp
    utils/*_test.cpp
)

add_executable(unit_tests
    main.cpp
    ${TEST_SOURCES}
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(unit_tests
    GTest::gtest
    GTest::gtest_main
    traderlib # Core library
    --coverage
)

include(GoogleTest)
gtest_discover_tests(unit_tests)

# Add coverage flags
# Code coverage doesn't work well with optimizations, so use -O0
target_compile_options(unit_tests PRIVATE -O0 -g --coverage)
