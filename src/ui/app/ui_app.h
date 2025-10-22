#pragma once

#include <quickfix/fix44/Message.h>

#include <cstdint>
#include <memory>
#include <thread>

#include "../../binance/config.h"
#include "../log_box/log_box.h"
#include "../order_book_box.h"
#include "../trade_box.h"
#include "../wallet_box.h"
#include "concurrentqueue.h"
#include "iscreen.h"

namespace ui {

// TODO(mils): login screen?

class App {
 public:
  explicit App(std::unique_ptr<IScreen> screen,
               std::unique_ptr<OrderBookBox> book_box,
               std::unique_ptr<LogBox> log_box,
               std::unique_ptr<TradeBox> trade_box);
  /// @brief start UI workers
  void start();
  /// if any exceptions occurred
  std::exception_ptr thread_exception;
  static App from_env(
      moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& order_queue,
      moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& trade_queue,
      binance::Config& binance_config);

 private:
  std::unique_ptr<IScreen> screen_;
  std::unique_ptr<OrderBookBox> book_box_;
  std::unique_ptr<LogBox> log_box_;
  std::unique_ptr<TradeBox> trade_box_;
  WalletBox wallet_box_;
};

}  // namespace ui
