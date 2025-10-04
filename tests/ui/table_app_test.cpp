#include <gtest/gtest.h>
#include <quickfix/fix44/Message.h>

#include <memory>
#include <mutex>
#include <string>

#include "concurrentqueue.h"
#include "core/order_book.h"
#include "fake_screen.h"
#include "spdlog/spdlog.h"
#include "ui/app.h"
#include "ui/iscreen.h"
#include "ui/log_box.h"
#include "ui/mock_log_reader.h"

TEST(UiApp, start) {
  // create app
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> order_queue{};
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> trade_queue{};

  std::unique_ptr<ui::IScreen> screen = std::make_unique<FakeScreen>();
  std::unique_ptr<ui::ILogReader> log_reader = std::make_unique<ui::MockLogReader>();
  auto task = ([](const std::stop_token& stoken) { spdlog::info("mock task"); });
  auto log_box = std::make_unique<ui::LogBox>(*screen.get(), std::move(log_reader), task);

  ui::App app{order_queue, trade_queue, std::move(screen), std::move(log_box)};
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
