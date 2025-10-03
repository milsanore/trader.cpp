#ifndef UI_TABLE_APP_H
#define UI_TABLE_APP_H

#include <quickfix/fix44/Message.h>

#include <memory>
#include <thread>

#include "FtxuiScreen.h"
#include "IScreen.h"
#include "LogBox.h"
#include "OrderBookBox.h"
#include "TradeBox.h"
#include "WalletBox.h"
#include "concurrentqueue.h"

namespace UI {

/*
login screen?
*/

class TableApp {
 public:
  explicit TableApp(
      moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &orderQueue,
      moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &tradeQueue,
      std::unique_ptr<IScreen> screen = std::make_unique<FtxuiScreen>());
  /// @brief start UI workers
  void start();
  /// if any exceptions occurred
  std::exception_ptr thread_exception;

 private:
  std::unique_ptr<IScreen> screen_;
  OrderBookBox book_;
  TradeBox trades_;
  WalletBox wallet_;
  LogBox logs_;
};

}  // namespace UI

#endif  // UI_TABLE_APP_H
