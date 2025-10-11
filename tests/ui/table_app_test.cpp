#include <gtest/gtest.h>
#include <quickfix/fix44/Message.h>

#include <memory>
#include <mutex>
#include <string>

#include "concurrentqueue.h"
#include "core/order_book.h"
#include "fake_screen.h"
#include "mock_log_watcher.h"
#include "spdlog/spdlog.h"
#include "ui/app/iscreen.h"
#include "ui/app/ui_app.h"
#include "ui/log_box/ilog_watcher.h"
#include "ui/log_box/log_box.h"

TEST(App, start) {
  // create app
  std::unique_ptr<ui::IScreen> screen = std::make_unique<FakeScreen>();

  std::unique_ptr<ui::ILogWatcher> log_reader = std::make_unique<ui::MockLogWatcher>();
  auto log_box = std::make_unique<ui::LogBox>(*screen.get(), std::move(log_reader));

  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> order_queue{};
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> trade_queue{};

  constexpr int MAX_DEPTH = 50;
  auto book_box = std::make_unique<ui::OrderBookBox>(*screen, order_queue, MAX_DEPTH);
  auto trade_box = std::make_unique<ui::TradeBox>(
      *screen, trade_queue,
      [](const std::stop_token& stoken) { spdlog::info("mock task"); });

  ui::App app = ui::App(order_queue, trade_queue, std::move(screen), std::move(book_box),
                        std::move(log_box), std::move(trade_box));
  app.start();

  // publish update
  FIX44::MarketDataSnapshotFullRefresh message;
  message.set(FIX::Symbol("BTCUSDT"));
  // Add a bid entry
  FIX44::MarketDataSnapshotFullRefresh::NoMDEntries bid;
  bid.set(FIX::MDEntryType(FIX::MDEntryType_BID));
  bid.set(FIX::MDEntryPx(95));
  bid.set(FIX::MDEntrySize(10));
  message.addGroup(bid);
  // Add an ask entry
  FIX44::MarketDataSnapshotFullRefresh::NoMDEntries ask;
  ask.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
  ask.set(FIX::MDEntryPx(96));
  ask.set(FIX::MDEntrySize(11));
  message.addGroup(ask);
  // push
  order_queue.enqueue(
      std::make_shared<const FIX44::MarketDataSnapshotFullRefresh>(message));

  // TODO: how to assert this? how to evaluate the UI change based on the input message?
}
