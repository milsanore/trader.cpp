#include <string>

#include "binance/Config.h"
#include "binance/Worker.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include "ui/TableApp.h"
#include "utils/Threading.h"

int main() {
  // LOG CONFIG
  spdlog::cfg::load_env_levels("LOG_LEVEL");
  const char* logPath = std::getenv("LOG_PATH");
  const auto logger = spdlog::basic_logger_mt("basic_logger", logPath);
  spdlog::set_default_logger(logger);
  // Set a global pattern without the logger name
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
  spdlog::flush_every(std::chrono::microseconds(100));

  spdlog::info("hello");
  Utils::Threading::set_thread_name("tradercppMAIN");

  // BINANCE MARKET DATA GENERATOR
  auto bConf = Binance::Config::fromEnv();
  auto bWorker = Binance::Worker::fromConf(bConf);
  bWorker.start();

  // UI APP (READS FROM QUEUE)
  auto app = UI::TableApp(bWorker.getOrderQueue(), bWorker.getTradeQueue());
  app.start();

  if (app.thread_exception) {
    std::rethrow_exception(app.thread_exception);
  }
  bWorker.stop();
  spdlog::info("goodbye");
}
