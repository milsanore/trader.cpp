#include <benchmark/benchmark.h>

#include <cstdlib>

#include "spdlog/spdlog.h"
#include "utils/logging.h"

struct Initializer {
  Initializer() {
    setenv("LOG_PATH", "logs/benchmark", 1);
    setenv("LOG_LEVEL", "info", 1);
    utils::Logging::configure();
  }
};
static Initializer init;

BENCHMARK_MAIN();
