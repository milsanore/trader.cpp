#pragma once

#include <quickfix/fix44/Message.h>

#include <memory>
#include <thread>

#include "IScreen.h"
#include "LogBox.h"
#include "OrderBookBox.h"
#include "TradeBox.h"
#include "WalletBox.h"
#include "concurrentqueue.h"

namespace ui {

/*
login screen?
*/

class TableApp {
 public:
  explicit TableApp(
      moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &orderQueue,
      moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &tradeQueue,
      std::unique_ptr<IScreen> screen, std::unique_ptr<LogBox> logs);
  /// @brief start UI workers
  void start();
  /// if any exceptions occurred
  std::exception_ptr thread_exception;
  static TableApp fromEnv(
      moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &orderQueue,
      moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &tradeQueue);

 private:
  std::unique_ptr<IScreen> screen_;
  OrderBookBox book_;
  TradeBox trades_;
  WalletBox wallet_;
  std::unique_ptr<LogBox> logs_;
};

}  // namespace ui
