#include "Worker.h"

#include <quickfix/Application.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>

#include <memory>

#include "./../utils/Threading.h"
#include "Auth.h"
#include "Config.h"
#include "FixApp.h"
#include "spdlog/spdlog.h"

namespace Binance {

Worker::Worker(std::unique_ptr<FixApp> app,
               std::unique_ptr<FIX::FileStoreFactory> fileStoreFactory,
               std::unique_ptr<FIX::SessionSettings> settings,
               std::unique_ptr<FIX::FileLogFactory> fileLogFactory,
               std::unique_ptr<FIX::SocketInitiator> initiator,
               const std::function<void(std::stop_token)> &task)
    : app_(std::move(app)),
      store_(std::move(fileStoreFactory)),
      settings_(std::move(settings)),
      log_(std::move(fileLogFactory)),
      initiator_(std::move(initiator)),
      workerTask_(task) {
  // default behaviour
  if (!task) {
    workerTask_ = ([this](const std::stop_token &stoken) {
      Utils::Threading::set_thread_name("tradercppFIX");
      // NB: SocketInitiator::start() is a blocking call, so the stop_token
      // cannot cancel the thread. NB: The `stop()` function has to forcibly
      // stop it with `initiator_->stop()`.
      initiator_->start();
      spdlog::info("started FIX session");
    });
  }
}

// static member function
Worker Worker::fromConf(Config &conf) {
  std::unique_ptr<IAuth> auth = std::make_unique<Auth>(conf.apiKey, conf.privateKeyPath);
  auto app = std::make_unique<FixApp>(conf.symbols, std::move(auth));
  auto settings = std::make_unique<FIX::SessionSettings>(conf.fixConfigPath);
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
    worker_ = std::jthread(workerTask_);
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

moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &Worker::getQueue()
    const {
  return app_->queue;
}

}  // namespace Binance
