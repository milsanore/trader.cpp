#include "worker.h"

#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>

#include <memory>

#include "./../utils/threading.h"
#include "auth.h"
#include "config.h"
#include "fix_app.h"
#include "spdlog/spdlog.h"

namespace binance {

Worker::Worker(std::unique_ptr<FixApp> app, std::unique_ptr<FIX::FileStoreFactory> store,
               std::unique_ptr<FIX::SessionSettings> settings,
               std::unique_ptr<FIX::FileLogFactory> log,
               std::unique_ptr<FIX::SocketInitiator> initiator,
               const std::function<void(std::stop_token)> &task)
    : app_(std::move(app)),
      store_(std::move(store)),
      settings_(std::move(settings)),
      log_(std::move(log)),
      initiator_(std::move(initiator)),
      worker_task_(task) {
  // default behaviour
  if (!task) {
    worker_task_ = ([this](const std::stop_token &stoken) {
      utils::Threading::set_thread_name("tradercppFIX");
      // NB: SocketInitiator::start() is a blocking call, so the stop_token
      // cannot cancel the thread. NB: The `stop()` function has to forcibly
      // stop it with `initiator_->stop()`.
      initiator_->start();
      spdlog::info("started FIX session");
    });
  }
}

// static member function
Worker Worker::from_conf(Config &conf) {
  std::unique_ptr<IAuth> auth =
      std::make_unique<Auth>(conf.api_key, conf.private_key_path);
  auto app = std::make_unique<FixApp>(conf.symbols, std::move(auth));
  auto settings = std::make_unique<FIX::SessionSettings>(conf.fix_config_path);
  auto store = std::make_unique<FIX::FileStoreFactory>(*settings);
  auto log = std::make_unique<FIX::FileLogFactory>(*settings);
  auto initiator = std::make_unique<FIX::SocketInitiator>(*app, *store, *settings, *log);

  // hand over object ownership to the instance being created (by the static
  // function)
  return {std::move(app), std::move(store), std::move(settings), std::move(log),
          std::move(initiator)};
}

void Worker::start() {
  try {
    worker_ = std::jthread(worker_task_);
  } catch (const std::exception &e) {
    spdlog::error("error starting binance FIX session, error [{}]", e.what());
  } catch (...) {
    spdlog::error("error starting binance FIX session, unknown error");
  }
}

void Worker::stop() {
  try {
    if (initiator_) {
      initiator_->stop();  // TODO(mils): does this need a try/catch?
    }
    spdlog::info("stopped FIX session");
  } catch (const std::exception &e) {
    spdlog::error("error stopping binance FIX session, error [{}]", e.what());
  } catch (...) {
    spdlog::error("error stopping binance FIX session, unknown error");
  }
  worker_ = std::jthread();
}

moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &
Worker::get_order_queue() const {
  return app_->order_queue_;
}

moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &
Worker::get_trade_queue() const {
  return app_->trade_queue_;
}

}  // namespace binance
