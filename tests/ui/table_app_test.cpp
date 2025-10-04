#include "ui/table_app.h"

#include <gtest/gtest.h>
#include <quickfix/fix44/Message.h>

#include <memory>
#include <mutex>
#include <string>

#include "concurrentqueue.h"
#include "core/order_book.h"
#include "fake_screen.h"
#include "spdlog/spdlog.h"
#include "ui/iscreen.h"
#include "ui/log_box.h"
#include "ui/mock_log_reader.h"

TEST(TableApp, start) {
  // create app
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> orderQueue{};
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> tradeQueue{};

  std::unique_ptr<ui::IScreen> screen = std::make_unique<FakeScreen>();
  std::unique_ptr<ui::ILogReader> logReader = std::make_unique<ui::MockLogReader>();
  auto task = ([](const std::stop_token& stoken) { spdlog::info("mock task"); });
  auto logBox = std::make_unique<ui::LogBox>(*screen.get(), std::move(logReader), task);

  ui::TableApp tblApp{orderQueue, tradeQueue, std::move(screen), std::move(logBox)};
  tblApp.start();

  // publish update
  FIX44::MarketDataSnapshotFullRefresh message;
  message.set(FIX::Symbol("BTCUSDT"));
  // Add a bid entry
  FIX44::MarketDataSnapshotFullRefresh::NoMDEntries bidEntry;
  bidEntry.set(FIX::MDEntryType(FIX::MDEntryType_BID));
  bidEntry.set(FIX::MDEntryPx(95));
  bidEntry.set(FIX::MDEntrySize(10));
  message.addGroup(bidEntry);
  // Add an ask entry
  FIX44::MarketDataSnapshotFullRefresh::NoMDEntries askEntry;
  askEntry.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
  askEntry.set(FIX::MDEntryPx(96));
  askEntry.set(FIX::MDEntrySize(11));
  message.addGroup(askEntry);
  // push
  orderQueue.enqueue(
      std::make_shared<const FIX44::MarketDataSnapshotFullRefresh>(message));

  // TODO: how to assert this? how to evaluate the UI change based on the input message?
}
