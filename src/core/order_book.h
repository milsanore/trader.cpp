#pragma once

#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>

#include <functional>
#include <map>
#include <mutex>

#include "bid_ask.h"

namespace core {

/// An order book class backed by two (synchronised) bid/ask maps
class OrderBook {
 public:
  explicit OrderBook(std::map<double, double, std::greater<>> bidMap = {},
                     std::map<double, double> askMap = {});

  // Mutex is not copyable:
  // 1. Delete copy constructor and copy assignment
  OrderBook(const OrderBook &) = delete;
  OrderBook &operator=(const OrderBook &) = delete;
  // 2. Declare move and move-assignment constructors
  OrderBook(OrderBook &&) noexcept;
  OrderBook &operator=(OrderBook &&) noexcept;

  void applySnapshot(const FIX44::MarketDataSnapshotFullRefresh &);
  void applyIncrement(const FIX44::MarketDataIncrementalRefresh &);
  /// @brief return the contents of the order book as a simple vector.
  /// useful for generating the UI
  std::vector<BidAsk> toVector();

 private:
  // mutex for reading/writing to bid/ask maps
  // NB: UI-bound, so performance is acceptable
  mutable std::mutex mutex_;
  /// @brief sorted list of bids (descending), key=price, value=size
  std::map<double, double, std::greater<>> bidMap_;
  /// @brief sorted list of offers (ascending), key=price, value=size
  std::map<double, double> askMap_;

  // std::vector<BidAsk> v(5000);
};

}  // namespace core
