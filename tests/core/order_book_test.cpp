#include "core/order_book.h"

#include <gtest/gtest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>

#include <cmath>
#include <string>

#include "absl/container/btree_map.h"
#include "core/bid_ask.h"

using core::BidAsk;

TEST(OrderBook, to_vector) {
  const absl::btree_map<uint64_t, uint64_t, std::greater<>> bids = {{95, 10}, {94, 9}};
  const absl::btree_map<uint64_t, uint64_t> asks = {
      {96, 11},
      {97, 12},
      {98, 13},
  };
  core::OrderBook x{bids, asks};
  const std::vector<BidAsk> ob_vec = x.to_vector();

  const std::vector check = {
      BidAsk{10, 95, 96, 11},
      BidAsk{9, 94, 97, 12},
      BidAsk{BidAsk::SENTINEL_, BidAsk::SENTINEL_, 98, 13},
  };
  ASSERT_EQ(ob_vec, check);

  const std::vector bad_check = {BidAsk{10, 95, 96, 11}, BidAsk{9, 94, 97, 12}};
  ASSERT_NE(ob_vec, bad_check);
}

TEST(OrderBook, constructors) {
  const absl::btree_map<uint64_t, uint64_t, std::greater<>> bids = {
      {95, 10},
  };
  const absl::btree_map<uint64_t, uint64_t> asks = {
      {96, 11},
  };
  core::OrderBook o1{bids, asks};
  // test move-constructor operator
  core::OrderBook moved1 = std::move(o1);

  const std::vector check = {BidAsk(10, 95, 96, 11)};
  ASSERT_EQ(moved1.to_vector(), check);

  // // test move-assignment operator
  // core::OrderBook moved2{};
  // moved2 = std::move(moved1);
  // ASSERT_EQ(moved2.to_vector(), check);
}

TEST(OrderBook, apply_snapshot) {
  FIX44::MarketDataSnapshotFullRefresh msg;
  // add a bid entry
  FIX44::MarketDataSnapshotFullRefresh::NoMDEntries bid;
  bid.set(FIX::MDEntryType(FIX::MDEntryType_BID));
  bid.set(FIX::MDEntryPx(95));
  bid.set(FIX::MDEntrySize(10));
  msg.addGroup(bid);
  // add an ask entry
  FIX44::MarketDataSnapshotFullRefresh::NoMDEntries ask;
  ask.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
  ask.set(FIX::MDEntryPx(96));
  ask.set(FIX::MDEntrySize(11));
  msg.addGroup(ask);
  // set the symbol - this will use production tick sizes (thus lots of zeroes below)
  msg.set(FIX::Symbol("BTCUSDT"));

  core::OrderBook book{};
  book.apply_snapshot(msg);
  const std::vector check = {BidAsk(1'000'000, 9500, 9600, 1'100'000)};
  ASSERT_EQ(book.to_vector(), check);
}

TEST(OrderBook, apply_increment) {
  absl::btree_map<uint64_t, uint64_t, std::greater<>> bids = {
      {9'500, 10'000'000},
  };
  absl::btree_map<uint64_t, uint64_t> asks = {
      {9'600, 11'000'000},
      {9'700, 12'000'000},
  };
  core::OrderBook book{bids, asks};

  FIX44::MarketDataIncrementalRefresh msg;
  // Add a bid update
  FIX44::MarketDataIncrementalRefresh::NoMDEntries bid;
  // set the symbol - this will use production tick sizes (thus lots of zeroes below)
  bid.set(FIX::Symbol("BTCUSDT"));
  bid.set(FIX::MDUpdateAction(FIX::MDUpdateAction_NEW));
  bid.set(FIX::MDEntryType(FIX::MDEntryType_BID));
  bid.set(FIX::MDEntryPx(95));
  bid.set(FIX::MDEntrySize(100));
  msg.addGroup(bid);
  // Add an ask delete
  FIX44::MarketDataIncrementalRefresh::NoMDEntries ask_delete;
  // set the symbol - this will use production tick sizes (thus lots of zeroes below)
  ask_delete.set(FIX::Symbol("BTCUSDT"));
  ask_delete.set(FIX::MDUpdateAction(FIX::MDUpdateAction_DELETE));
  ask_delete.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
  ask_delete.set(FIX::MDEntryPx(97));
  msg.addGroup(ask_delete);

  constexpr bool IS_BOOK_CLEAR_NEEDED_ = false;
  book.apply_increment(msg, IS_BOOK_CLEAR_NEEDED_);
  std::vector<BidAsk> vec = book.to_vector();
  std::vector check = {BidAsk(10'000'000, 9'500, 9'600, 11'000'000)};
  ASSERT_EQ(vec, check);
}
