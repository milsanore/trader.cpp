#pragma once

#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>

#include <cstdint>
#include <functional>
#include <mutex>

#include "../binance/symbol.h"
#include "absl/container/btree_map.h"
#include "bid_ask.h"

namespace core {

/// An order book class backed by two (synchronised) bid/ask maps
class OrderBook {
 public:
  explicit OrderBook(absl::btree_map<uint64_t, uint64_t, std::greater<>> bid_map = {},
                     absl::btree_map<uint64_t, uint64_t> ask_map = {});

  // Mutex is not copyable:
  // 1. Delete copy constructor and copy assignment
  OrderBook(const OrderBook&) = delete;
  OrderBook& operator=(const OrderBook&) = delete;
  // 2. Declare move and move-assignment constructors
  OrderBook(OrderBook&&) noexcept;
  // OrderBook& operator=(OrderBook&&) noexcept;

  void apply_snapshot(const FIX44::MarketDataSnapshotFullRefresh&);
  void apply_increment(const FIX44::MarketDataIncrementalRefresh&,
                       bool is_book_clear_needed);
  /// @brief return the contents of the order book as a simple vector.
  /// useful for generating the UI
  std::vector<BidAsk> to_vector();

 private:
  // mutex for reading/writing to bid/ask maps
  // NB: UI-bound, so performance is acceptable
  mutable std::mutex mutex_;
  /// @brief sorted list of bids (descending), key=price, value=size
  absl::btree_map<uint64_t, uint64_t, std::greater<>> bid_map_;
  /// @brief sorted list of offers (ascending), key=price, value=size
  absl::btree_map<uint64_t, uint64_t> ask_map_;
  //
  FIX::MDEntryPx e_px_;
  FIX::MDEntrySize e_sz_;
  inline void handle_price_level_update(
      auto& bid_ask_map,
      binance::SymbolEnum symbol,
      FIX::MDUpdateAction action,
      const FIX44::MarketDataIncrementalRefresh::NoMDEntries& group,
      bool is_book_clear_needed);
};

}  // namespace core
