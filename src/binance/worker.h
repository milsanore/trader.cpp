#pragma once

#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include <quickfix/fix44/Message.h>

#include <memory>
#include <thread>

#include "concurrentqueue.h"
#include "config.h"
#include "fix_app.h"

namespace binance {

/// @brief Binance DI container
class Worker final {
 public:
  static constexpr std::string THREAD_NAME_ = "tradercppFIX1";
  Worker(std::unique_ptr<FixApp> app,
         std::unique_ptr<FIX::FileStoreFactory> store,
         FIX::SessionSettings settings,
         std::unique_ptr<FIX::FileLogFactory> log,
         std::unique_ptr<FIX::SocketInitiator> initiator,
         const std::function<void(std::stop_token)>& task = {});
  /// @brief factory for concrete Binance instances, using config
  /// @param conf Binance configuration parameters
  /// @return
  static Worker from_conf(Config& conf);
  /// @brief start worker thread, connect to Binance, subscribe to updates, push
  /// updates onto queue. under the hood the {FixApp} is actioned
  void start();
  void stop();
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& get_order_queue()
      const;
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& get_trade_queue()
      const;

 private:
  // FIX
  std::unique_ptr<FixApp> app_;
  std::unique_ptr<FIX::FileStoreFactory> store_;
  FIX::SessionSettings settings_;
  std::unique_ptr<FIX::FileLogFactory> log_;
  std::unique_ptr<FIX::SocketInitiator> initiator_;
  // thread
  std::jthread worker_;
  std::function<void(std::stop_token)> worker_task_;
};

}  // namespace binance
