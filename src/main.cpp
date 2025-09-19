#include <chrono>
#include <string>
#include "binance/Config.h"
#include "binance/Worker.h"
#include "ui/TableApp.h"
#include "spdlog/cfg/env.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

int main() {
    spdlog::cfg::load_env_levels("LOG_LEVEL");
    const auto logger = spdlog::basic_logger_mt("basic_logger", "logs/log");
    spdlog::set_default_logger(logger);

    spdlog::info("hello");

    // BINANCE MARKET DATA GENERATOR
    auto bConf = Binance::Config::fromEnv();
	auto bWorker = Binance::Worker::fromConf(bConf);
    bWorker.start();

    // UI APP (READS FROM QUEUE)
    auto app = UI::TableApp(bWorker.app->queue);
    app.start();

    if (app.thread_exception) {
        std::rethrow_exception(app.thread_exception);
    }
    bWorker.stop();
    spdlog::info("goodbye");
}
