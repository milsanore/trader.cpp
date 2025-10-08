#include "core/order_book.h"

#include <gtest/gtest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>

#include <cmath>
#include <map>
#include <string>

#include "core/bid_ask.h"

TEST(OrderBook, to_vector) {
  std::map<double, double, std::greater<>> bids = {{95, 10}, {94, 9}};
  std::map<double, double> asks = {
      {96, 11},
      {97, 12},
      {98, 13},
  };
  core::OrderBook x{bids, asks};
  std::vector<core::BidAsk> ob_vec = x.to_vector();

  std::vector check = {
      core::BidAsk{10, 95, 96, 11},
      core::BidAsk{9, 94, 97, 12},
      core::BidAsk{NAN, NAN, 98, 13},
  };
  ASSERT_EQ(ob_vec, check);

  std::vector bad_check = {core::BidAsk{10, 95, 96, 11}, core::BidAsk{9, 94, 97, 12}};
  ASSERT_NE(ob_vec, bad_check);
}

TEST(OrderBook, constructors) {
  std::map<double, double, std::greater<>> bids = {
      {95, 10},
  };
  std::map<double, double> asks = {
      {96, 11},
  };
  core::OrderBook o1{bids, asks};
  // test move-constructor operator
  core::OrderBook moved1 = std::move(o1);

  std::vector<core::BidAsk> check = {{10, 95, 96, 11}};
  ASSERT_EQ(moved1.to_vector(), check);

  core::OrderBook moved2{};
  // test move-assignment operator
  moved2 = std::move(moved1);
  ASSERT_EQ(moved2.to_vector(), check);
}

TEST(OrderBook, apply_snapshot) {
  core::OrderBook book{};

  FIX44::MarketDataSnapshotFullRefresh msg;
  msg.set(FIX::Symbol("BTCUSDT"));
  // Add a bid entry
  FIX44::MarketDataSnapshotFullRefresh::NoMDEntries bid;
  bid.set(FIX::MDEntryType(FIX::MDEntryType_BID));
  bid.set(FIX::MDEntryPx(95));
  bid.set(FIX::MDEntrySize(10));
  msg.addGroup(bid);
  // Add an ask entry
  FIX44::MarketDataSnapshotFullRefresh::NoMDEntries ask;
  ask.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
  ask.set(FIX::MDEntryPx(96));
  ask.set(FIX::MDEntrySize(11));
  msg.addGroup(ask);

  book.apply_snapshot(msg);
  std::vector<core::BidAsk> check = {{10, 95, 96, 11}};
  ASSERT_EQ(book.to_vector(), check);
}

TEST(OrderBook, apply_increment) {
  std::map<double, double, std::greater<>> bids = {
      {95, 10},
  };
  std::map<double, double> asks = {
      {96, 11},
      {97, 12},
  };
  core::OrderBook book{bids, asks};

  FIX44::MarketDataIncrementalRefresh msg;
  // Add a bid update
  FIX44::MarketDataIncrementalRefresh::NoMDEntries bid;
  bid.set(FIX::Symbol("BTCUSDT"));
  bid.set(FIX::MDUpdateAction(FIX::MDUpdateAction_NEW));
  bid.set(FIX::MDEntryType(FIX::MDEntryType_BID));
  bid.set(FIX::MDEntryPx(95));
  bid.set(FIX::MDEntrySize(100));
  msg.addGroup(bid);
  // Add an ask delete
  FIX44::MarketDataIncrementalRefresh::NoMDEntries ask_delete;
  ask_delete.set(FIX::Symbol("BTCUSDT"));
  ask_delete.set(FIX::MDUpdateAction(FIX::MDUpdateAction_DELETE));
  ask_delete.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
  ask_delete.set(FIX::MDEntryPx(97));
  msg.addGroup(ask_delete);

  book.apply_increment(msg);
  std::vector<core::BidAsk> vec = book.to_vector();
  std::vector<core::BidAsk> check = {{100, 95, 96, 11}};
  ASSERT_EQ(vec, check);
}
