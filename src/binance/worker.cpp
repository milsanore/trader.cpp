#include "worker.h"

#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/ThreadedSocketInitiator.h>
#include <quickfix/fix44/MarketDataIncrementalRefresh.h>

#include <memory>

#include "../utils/threading.h"
#include "auth.h"
#include "config.h"
#include "fix_app.h"
#include "market_message_variant.h"
#include "spdlog/spdlog.h"

namespace binance {

Worker::Worker(std::unique_ptr<FixApp> app,
               std::unique_ptr<FIX::FileStoreFactory> store,
               FIX::SessionSettings settings,
               std::unique_ptr<FIX::FileLogFactory> log,
               std::unique_ptr<FIX::ThreadedSocketInitiator> initiator)
    : app_(std::move(app)),
      store_(std::move(store)),
      settings_(std::move(settings)),
      log_(std::move(log)),
      initiator_(std::move(initiator)) {}

// static member function
Worker Worker::from_conf(Config& conf) {
  std::unique_ptr<IAuth> auth =
      std::make_unique<Auth>(conf.api_key, conf.private_key_path);
  auto app = std::make_unique<FixApp>(conf.symbols, std::move(auth), conf.MAX_DEPTH,
                                      conf.px_cpu, conf.tx_cpu);
  auto settings = FIX::SessionSettings{conf.fix_config_path};
  auto store = std::make_unique<FIX::FileStoreFactory>(settings);
  auto log = std::make_unique<FIX::FileLogFactory>(settings);
  auto initiator =
      std::make_unique<FIX::ThreadedSocketInitiator>(*app, *store, settings, *log);

  // hand over object ownership to the instance being created (by the static
  // function)
  return {std::move(app), std::move(store), std::move(settings), std::move(log),
          std::move(initiator)};
}

void Worker::start() {
  initiator_->start();
  spdlog::info("started FIX initiator");
}

void Worker::stop() {
  initiator_->stop();  // TODO(mils): does this need a try/catch?
  spdlog::info("stopped FIX initiator");
}

moodycamel::ConcurrentQueue<MarketMessageVariant>& Worker::get_order_queue() const {
  return app_->order_queue_;
}

moodycamel::ConcurrentQueue<FIX44::MarketDataIncrementalRefresh>&
Worker::get_trade_queue() const {
  return app_->trade_queue_;
}

}  // namespace binance
