file(GLOB_RECURSE BENCHMARK_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(benchmarks ${BENCHMARK_SOURCES})

target_include_directories(benchmarks PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(benchmarks PRIVATE
    traderlib  # Core library
    abseil::abseil
    benchmark::benchmark
    spdlog::spdlog
)

add_test(NAME benchmarks COMMAND benchmarks)

set_tests_properties(benchmarks PROPERTIES LABELS "BENCHMARK")
