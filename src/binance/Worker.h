#ifndef BINANCE_WORKER_H
#define BINANCE_WORKER_H

#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include <quickfix/fix44/Message.h>

#include <memory>
#include <thread>

#include "Config.h"
#include "FixApp.h"
#include "concurrentqueue.h"

namespace Binance {

/// @brief Binance DI container
class Worker final {
 public:
  Worker(std::unique_ptr<FixApp> fixApp,
         std::unique_ptr<FIX::FileStoreFactory> fileStoreFactory,
         std::unique_ptr<FIX::SessionSettings> settings,
         std::unique_ptr<FIX::FileLogFactory> fileLogFactory,
         std::unique_ptr<FIX::SocketInitiator> initiator,
         std::function<void(std::stop_token)> task = {});
  /// @brief factory for concrete Binance instances, using config
  /// @param conf binance configuration parameters
  /// @return
  static Worker fromConf(Config &conf);
  /// @brief start worker thread, connect to Binance, subscribe to updates, push
  /// updates onto queue. under the hood the {FixApp} is actioned
  void start();
  void stop();
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &getQueue() const;

 private:
  // worker thread
  std::jthread worker_;
  std::function<void(std::stop_token)> workerTask_;
  // FIX
  std::unique_ptr<FixApp> app_;
  std::unique_ptr<FIX::FileStoreFactory> store_;
  std::unique_ptr<FIX::SessionSettings> settings_;
  std::unique_ptr<FIX::FileLogFactory> log_;
  std::unique_ptr<FIX::SocketInitiator> initiator_;
};

}  // namespace Binance

#endif  // BINANCE_WORKER_H
