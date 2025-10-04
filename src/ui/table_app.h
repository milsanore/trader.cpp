#pragma once

#include <quickfix/fix44/Message.h>

#include <memory>
#include <thread>

#include "concurrentqueue.h"
#include "iscreen.h"
#include "log_box.h"
#include "order_book_box.h"
#include "trade_box.h"
#include "wallet_box.h"

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
