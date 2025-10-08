#include "order_book.h"

#include "bid_ask.h"
#include "spdlog/spdlog.h"

namespace core {

OrderBook::OrderBook(std::map<double, double, std::greater<>> bid_map,
                     std::map<double, double> ask_map)
    : bid_map_(std::move(bid_map)), ask_map_(std::move(ask_map)) {}

// move constructor
OrderBook::OrderBook(OrderBook&& other) noexcept
    : bid_map_(std::move(other.bid_map_)), ask_map_(std::move(other.ask_map_)) {
  // lock other.mutex_ to ensure safe access to its internal maps while moving
  std::lock_guard lock(other.mutex_);

  // mutex_ does not move; each instance has its own mutex
}

// move-assignment constructor
OrderBook& OrderBook::operator=(OrderBook&& other) noexcept {
  if (this != &other) {
    // Lock both mutexes without deadlock
    std::scoped_lock lock(mutex_, other.mutex_);
    bid_map_ = std::move(other.bid_map_);
    ask_map_ = std::move(other.ask_map_);
  }
  return *this;
}
std::vector<BidAsk> OrderBook::to_vector() {
  std::lock_guard lock(mutex_);
  const size_t row_count = std::max(bid_map_.size(), ask_map_.size());
  // TODO(mils): could this be a pre-allocated/reusable vector with exactly 5000
  // entries?
  std::vector<BidAsk> v(row_count);
  // bids
  int i = 0;
  for (const auto& [px, sz] : bid_map_) {
    v[i].bid_sz = sz;
    v[i].bid_px = px;
    i++;
  }
  // asks
  i = 0;
  for (const auto& [px, sz] : ask_map_) {
    v[i].ask_sz = sz;
    v[i].ask_px = px;
    i++;
  }
  return v;
};

void OrderBook::apply_snapshot(const FIX44::MarketDataSnapshotFullRefresh& msg) {
  std::lock_guard lock(mutex_);
  FIX::Symbol symbol;
  msg.get(symbol);
  spdlog::debug("MD snapshot message, symbol [{}]", symbol.getString());

  if (std::string s = symbol.getValue(); s != "BTCUSDT") {
    spdlog::debug("wrong symbol, skipping snapshot. value [{}]", s);
    return;
  }

  bid_map_.clear();
  ask_map_.clear();
  FIX::NoMDEntries entries;
  msg.get(entries);
  const int num_entries = entries.getValue();
  for (int i = 1; i <= num_entries; i++) {
    FIX44::MarketDataSnapshotFullRefresh::NoMDEntries group;
    msg.getGroup(i, group);
    FIX::MDEntryType entry_type;
    FIX::MDEntryPx px;
    FIX::MDEntrySize sz;
    group.get(entry_type);
    group.get(px);
    group.get(sz);
    if (entry_type == FIX::MDEntryType_BID) {
      bid_map_[px.getValue()] = sz.getValue();
    } else if (entry_type == FIX::MDEntryType_OFFER) {
      ask_map_[px.getValue()] = sz.getValue();
    } else {
      spdlog::error(std::format("unknown bid/offer type [{}]", entry_type.getString()));
    }
  }
}

void OrderBook::apply_increment(const FIX44::MarketDataIncrementalRefresh& msg) {
  std::lock_guard lock(mutex_);
  FIX::NoMDEntries entries;
  msg.get(entries);
  const int num_entries = entries.getValue();
  std::string symbol;
  for (int i = 1; i <= num_entries; i++) {
    FIX44::MarketDataIncrementalRefresh::NoMDEntries group;
    msg.getGroup(i, group);

    // if symbol unchanged, field won't be set
    if (group.isSetField(FIX::FIELD::Symbol)) {
      FIX::Symbol smbl;
      group.get(smbl);
      if (std::string s = smbl.getValue(); !s.empty()) {
        symbol = s;
      }
    }
    if (symbol.empty() && symbol != "BTCUSDT") {
      spdlog::debug("wrong symbol, skipping increment. value [{}]", symbol);
      continue;
    }

    FIX::MDUpdateAction action;
    FIX::MDEntryType entry_type;
    FIX::MDEntryPx px;
    group.get(action);
    group.get(entry_type);
    group.get(px);

    if (entry_type != FIX::MDEntryType_BID && entry_type != FIX::MDEntryType_OFFER) {
      spdlog::error(std::format("unknown entry type, skipping. value [{}]",
                                entry_type.getString()));
      continue;
    }

    if (action.getValue() == FIX::MDUpdateAction_NEW ||
        action.getValue() == FIX::MDUpdateAction_CHANGE) {
      spdlog::debug("price upsert");

      if (group.isSetField(FIX::FIELD::MDEntrySize)) {
        FIX::MDEntrySize sz;
        group.get(sz);
        if (entry_type == FIX::MDEntryType_BID) {
          bid_map_[px.getValue()] = sz.getValue();
        } else if (entry_type == FIX::MDEntryType_OFFER) {
          ask_map_[px.getValue()] = sz.getValue();
        }
      }
    } else if (action.getValue() == FIX::MDUpdateAction_DELETE) {
      spdlog::debug("price delete");

      if (entry_type == FIX::MDEntryType_BID) {
        bid_map_.erase(px.getValue());
      } else if (entry_type == FIX::MDEntryType_OFFER) {
        ask_map_.erase(px.getValue());
      }
    } else {
      spdlog::error(std::format("unknown price action. value [{}]", action.getValue()));
    }
  }
}

}  // namespace core
