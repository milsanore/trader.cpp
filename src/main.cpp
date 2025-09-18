#include <chrono>
#include <format>
#include <iostream>
#include <string>
#include <thread>
#include "concurrentqueue.h"
#include "binance/Config.h"
#include "binance/Init.h"
#include "ui/TableApp.h"
#include "spdlog/cfg/env.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

int main() {
    spdlog::cfg::load_env_levels("LOG_LEVEL");
    auto logger = spdlog::basic_logger_mt("basic_logger", "logs/log");
    spdlog::set_default_logger(logger);

    spdlog::info("hello");

    // BINANCE MARKET DATA GENERATOR
    auto bConf = Binance::Config::fromEnv();
	auto binance = Binance::Init::fromConf(bConf);
    binance.start();

    // UI APP (READS FROM QUEUE)
    UI::TableApp app = UI::TableApp(binance.app->queue);
    app.start();

    if (app.thread_exception) {
        std::rethrow_exception(app.thread_exception);
    }
    binance.stop();
    spdlog::info("goodbye");
}
