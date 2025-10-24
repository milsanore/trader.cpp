#include "order_book.h"

#include "../binance/config.h"
#include "../binance/symbol.h"
#include "../utils/double.h"
#include "absl/container/btree_map.h"
#include "bid_ask.h"
#include "spdlog/spdlog.h"

namespace core {

OrderBook::OrderBook(absl::btree_map<uint64_t, uint64_t, std::greater<>> bid_map,
                     absl::btree_map<uint64_t, uint64_t> ask_map)
    : bid_map_(std::move(bid_map)), ask_map_(std::move(ask_map)) {}

// move constructor
OrderBook::OrderBook(OrderBook&& other) noexcept
    : bid_map_(std::move(other.bid_map_)), ask_map_(std::move(other.ask_map_)) {
  // lock other.mutex_ to ensure safe access to its internal maps while moving
  std::lock_guard lock(other.mutex_);

  // mutex_ does not move; each instance has its own mutex
}

// move-assignment constructor
// OrderBook& OrderBook::operator=(OrderBook&& other) noexcept {
//   if (this != &other) {
//     // Lock both mutexes without deadlock
//     std::scoped_lock lock(mutex_, other.mutex_);
//     bid_map_ = std::move(other.bid_map_);
//     ask_map_ = std::move(other.ask_map_);
//   }
//   return *this;
// }

std::vector<BidAsk> OrderBook::to_vector() {
  std::lock_guard lock(mutex_);
  const size_t row_count = std::max(bid_map_.size(), ask_map_.size());
  std::vector<BidAsk> v;
  v.reserve(row_count);

  auto bid_it = bid_map_.begin();
  auto ask_it = ask_map_.begin();

  for (; bid_it != bid_map_.end() || ask_it != ask_map_.end();) {
    BidAsk ba{};
    if (bid_it != bid_map_.end()) {
      ba.bid_px = bid_it->first;
      ba.bid_sz = bid_it->second;
      ++bid_it;
    }
    if (ask_it != ask_map_.end()) {
      ba.ask_px = ask_it->first;
      ba.ask_sz = ask_it->second;
      ++ask_it;
    }
    v.push_back(std::move(ba));
  }

  return v;
}

void OrderBook::apply_snapshot(const FIX44::MarketDataSnapshotFullRefresh& msg) {
  std::lock_guard lock(mutex_);
  FIX::Symbol symbol;
  msg.get(symbol);
  spdlog::info("MD snapshot message. symbol [{}]", symbol.getString());

  binance::SymbolEnum sym = binance::Symbol::from_str(symbol.getValue());
  if (sym != binance::SymbolEnum::BTCUSDT) {
    spdlog::error("wrong symbol, skipping snapshot. value [{}]",
                  binance::Symbol::to_str(sym));
    return;
  }

  bid_map_.clear();
  ask_map_.clear();

  FIX::NoMDEntries entries;
  msg.get(entries);
  const int num_entries = entries.getValue();
  FIX44::MarketDataSnapshotFullRefresh::NoMDEntries group;
  FIX::MDEntryType e_tp;
  FIX::MDEntryPx e_px;
  FIX::MDEntrySize e_sz;
  for (int i = 1; i <= num_entries; i++) {
    msg.getGroup(i, group);
    group.get(e_tp);
    group.get(e_px);
    group.get(e_sz);

    uint64_t px = utils::Double::toUint64(e_px.getValue(),
                                          binance::Config::get_price_ticks_per_unit(sym));

    uint64_t sz = utils::Double::toUint64(e_sz.getValue(),
                                          binance::Config::get_size_ticks_per_unit(sym));

    if (e_tp == FIX::MDEntryType_BID) {
      bid_map_[px] = sz;
    } else if (e_tp == FIX::MDEntryType_OFFER) {
      ask_map_[px] = sz;
    } else {
      spdlog::error("unknown bid/offer type [{}]", e_tp.getString());
    }
  }
}

void OrderBook::apply_increment(const FIX44::MarketDataIncrementalRefresh& msg,
                                bool is_book_clear_needed) {
  std::lock_guard lock(mutex_);
  FIX::NoMDEntries entries;
  msg.get(entries);
  const int num_entries = entries.getValue();

  FIX44::MarketDataIncrementalRefresh::NoMDEntries group;
  FIX::Symbol fsym;
  std::optional<binance::SymbolEnum> symbol;
  FIX::MDEntryType e_tp;
  FIX::MDUpdateAction action;
  for (int i = 1; i <= num_entries; i++) {
    msg.getGroup(i, group);

    // Update symbol if present or first group
    if (i == 1 || group.isSetField(FIX::FIELD::Symbol)) {
      group.get(fsym);
      symbol = binance::Symbol::from_str(fsym.getValue());
    }
    if (!symbol) {
      spdlog::error("missing symbol, skipping price increment. value [{}]");
      continue;
    }
    // debug
    if (symbol != binance::SymbolEnum::BTCUSDT) {
      spdlog::error("wrong symbol, skipping price increment. value [{}]",
                    binance::Symbol::to_str(symbol.value()));
      continue;
    }

    group.get(e_tp);
    group.get(action);
    switch (e_tp.getValue()) {
      case FIX::MDEntryType_BID:
        handle_price_level_update(bid_map_, symbol.value(), action, group,
                                  is_book_clear_needed);
        break;
      case FIX::MDEntryType_OFFER:
        handle_price_level_update(ask_map_, symbol.value(), action, group,
                                  is_book_clear_needed);
        break;
      default:
        spdlog::error("unknown bid/offer FIX::MDEntryType. value [{}]", e_tp.getValue());
    }
  }
}

void OrderBook::handle_price_level_update(
    auto& bid_ask_map,
    binance::SymbolEnum symbol,
    FIX::MDUpdateAction action,
    const FIX44::MarketDataIncrementalRefresh::NoMDEntries& group,
    bool is_book_clear_needed) {
  uint64_t px =
      utils::Double::toUint64(group.get(temp_vars_.e_px).getValue(),
                              binance::Config::get_price_ticks_per_unit(symbol));
  //
  switch (action.getValue()) {
    case FIX::MDUpdateAction_DELETE:
      bid_ask_map.erase(px);
      break;
    case FIX::MDUpdateAction_CHANGE: {
      if (is_book_clear_needed) {
        bid_ask_map.clear();
      }
      [[fallthrough]];
    }
    case FIX::MDUpdateAction_NEW: {
      uint64_t sz =
          utils::Double::toUint64(group.get(temp_vars_.e_sz).getValue(),
                                  binance::Config::get_size_ticks_per_unit(symbol));
      bid_ask_map[px] = sz;
    } break;
    default:
      spdlog::error("unknown price action. value [{}]", action.getValue());
  }
};

}  // namespace core
