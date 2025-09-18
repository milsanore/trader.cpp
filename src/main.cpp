#include <chrono>
#include <format>
#include <iostream>
#include <string>
#include <thread>
#include "concurrentqueue.h"
#include "binance/Config.h"
#include "binance/Init.h"
#include "ui/TableApp.h"

int main() {
    std::cout << std::format("hello") << std::endl;

    // BINANCE MARKET DATA GENERATOR
    auto bConf = Binance::Config::fromEnv();
	auto binance = Binance::Init::fromConf(bConf);
    binance.start();

    // UI APP
    UI::TableApp app = UI::TableApp(binance.app->queue);
    app.start();

    if (app.thread_exception) {
        std::rethrow_exception(app.thread_exception);
    }
    binance.stop();
    std::cout << std::format("goodbye") << std::endl;
}
