#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <new>
#include <string>

#include "binance/config.h"
#include "binance/worker.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include "ui/app/ui_app.h"
#include "utils/crash.h"
#include "utils/env.h"
#include "utils/threading.h"

int main() {
  try {
    utils::Threading::set_thread_name("main");

    // log config
    spdlog::cfg::load_env_levels("LOG_LEVEL");
    const char* log_path = std::getenv("LOG_PATH");
    const auto logger = spdlog::basic_logger_mt("basic_logger", log_path);
    spdlog::set_default_logger(logger);
    // Set a global pattern without the logger name
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
    spdlog::flush_every(std::chrono::microseconds(100));
    spdlog::info("hello");
    utils::Env::log_current_architecture();

    // configure error-handling
    utils::Crash::setup_crash_handlers();

    // Binance market data connectivity
    auto b_conf = binance::Config::from_env();
    auto b_worker = binance::Worker::from_conf(b_conf);
    b_worker.start();

    // ui app (reads from Binance's queues)
    auto ui =
        ui::App::from_env(b_worker.get_order_queue(), b_worker.get_trade_queue(), b_conf);
    // blocking
    ui.start();

    if (ui.thread_exception) {
      std::rethrow_exception(ui.thread_exception);
    }
    b_worker.stop();
    spdlog::info("goodbye");
  } catch (const std::exception& e) {
    spdlog::error("[EXCEPTION] Caught exception. ex [{}]", e.what());
  } catch (...) {
    spdlog::error("[EXCEPTION] Caught unknown exception");
  }
}
