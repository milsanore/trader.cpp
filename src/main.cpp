#include <exception>
#include <string>

#include "binance/config.h"
#include "binance/worker.h"
#include "spdlog/spdlog.h"
#include "ui/app/ui_app.h"
#include "utils/crash.h"
#include "utils/logging.h"
#include "utils/process.h"
#include "utils/threading.h"

int main() {
  try {
    utils::Threading::set_thread_name("main");
    utils::Logging::configure();
    utils::Crash::configure_handlers();
    utils::Process::set_high_priority();

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
    spdlog::critical("[EXCEPTION] Caught exception. ex [{}]", e.what());
  } catch (...) {
    spdlog::critical("[EXCEPTION] Caught unknown exception");
  }
}
