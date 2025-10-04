#include <gtest/gtest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>

#include <cmath>
#include <map>
#include <string>

#include "ui/BidAsk.h"
#include "ui/OrderBook.h"

TEST(OrderBook, toVector) {
  std::map<double, double, std::greater<>> bids = {{95, 10}, {94, 9}};
  std::map<double, double> asks = {
      {96, 11},
      {97, 12},
      {98, 13},
  };
  ui::OrderBook x{bids, asks};
  std::vector<ui::BidAsk> obVec = x.toVector();

  std::vector check = {
      ui::BidAsk{10, 95, 96, 11},
      ui::BidAsk{9, 94, 97, 12},
      ui::BidAsk{NAN, NAN, 98, 13},
  };
  ASSERT_EQ(obVec, check);

  std::vector badCheck = {ui::BidAsk{10, 95, 96, 11}, ui::BidAsk{9, 94, 97, 12}};
  ASSERT_NE(obVec, badCheck);
}

TEST(OrderBook, constructors) {
  std::map<double, double, std::greater<>> bids = {
      {95, 10},
  };
  std::map<double, double> asks = {
      {96, 11},
  };
  ui::OrderBook o1{bids, asks};
  // test move-constructor operator
  ui::OrderBook moved1 = std::move(o1);

  std::vector<ui::BidAsk> check = {{10, 95, 96, 11}};
  ASSERT_EQ(moved1.toVector(), check);

  ui::OrderBook moved2{};
  // test move-assignment operator
  moved2 = std::move(moved1);
  ASSERT_EQ(moved2.toVector(), check);
}

TEST(OrderBook, applySnapshot) {
  ui::OrderBook book{};

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

  book.applySnapshot(message);
  std::vector<ui::BidAsk> check = {{10, 95, 96, 11}};
  ASSERT_EQ(book.toVector(), check);
}

TEST(OrderBook, applyIncrement) {
  std::map<double, double, std::greater<>> bids = {
      {95, 10},
  };
  std::map<double, double> asks = {
      {96, 11},
      {97, 12},
  };
  ui::OrderBook book{bids, asks};

  FIX44::MarketDataIncrementalRefresh message;
  // Add a bid update
  FIX44::MarketDataIncrementalRefresh::NoMDEntries bidEntry;
  bidEntry.set(FIX::Symbol("BTCUSDT"));
  bidEntry.set(FIX::MDUpdateAction(FIX::MDUpdateAction_NEW));
  bidEntry.set(FIX::MDEntryType(FIX::MDEntryType_BID));
  bidEntry.set(FIX::MDEntryPx(95));
  bidEntry.set(FIX::MDEntrySize(100));
  message.addGroup(bidEntry);
  // Add an ask delete
  FIX44::MarketDataIncrementalRefresh::NoMDEntries askDelete;
  askDelete.set(FIX::Symbol("BTCUSDT"));
  askDelete.set(FIX::MDUpdateAction(FIX::MDUpdateAction_DELETE));
  askDelete.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
  askDelete.set(FIX::MDEntryPx(97));
  message.addGroup(askDelete);

  book.applyIncrement(message);
  std::vector<ui::BidAsk> vec = book.toVector();
  std::vector<ui::BidAsk> check = {{100, 95, 96, 11}};
  ASSERT_EQ(vec, check);
}