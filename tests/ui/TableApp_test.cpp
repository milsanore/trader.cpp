#include "ui/TableApp.h"

#include <gtest/gtest.h>
#include <quickfix/fix44/Message.h>

#include <mutex>
#include <string>

#include "FakeScreen.h"
#include "concurrentqueue.h"
#include "spdlog/spdlog.h"
#include "ui/IScreen.h"
#include "ui/OrderBook.h"

TEST(TableApp, start) {
  // create app
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> queue{};
  auto book = std::make_unique<UI::OrderBook>();
  std::unique_ptr<UI::IScreen> screen = std::make_unique<FakeScreen>();
  UI::TableApp tblApp{queue, std::move(book), std::move(screen)};
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
  queue.enqueue(std::make_shared<const FIX44::MarketDataSnapshotFullRefresh>(message));

  // TODO: how to assert this? how to evaluate the UI change based on the input message?
}
