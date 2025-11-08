#include "env.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace utils {

/// @brief logger helpers
struct Logging {
 public:
  static void configure() {
    // basic sink for general logging (file)
    const char* log_path = std::getenv("LOG_PATH");
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path, false);
    file_sink->set_level(spdlog::level::info);
    // additional sink for `critical` logging to console (as a convenience)
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdout_sink->set_level(spdlog::level::critical);
    // combine sinks
    auto logger = std::make_shared<spdlog::logger>(
        "multi_sink", spdlog::sinks_init_list{file_sink, stdout_sink});
    spdlog::set_default_logger(logger);
    // read the `LOG_LEVEL` envar for the minimum severity to log (at the logger level)
    spdlog::cfg::load_env_levels("LOG_LEVEL");
    // Set a global pattern without the logger name
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
    spdlog::flush_every(std::chrono::seconds(1));
    spdlog::info("hello");
    utils::Env::log_current_architecture();
  }
};
}  // namespace utils
