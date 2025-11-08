#include <benchmark/benchmark.h>
#include <fmt/ranges.h>

#include <random>

#include "core/bid_ask.h"
#include "core/order_book.h"
#include "spdlog/spdlog.h"

class OrderBookFixture : public benchmark::Fixture {
 public:
  void SetUp([[maybe_unused]] const benchmark::State& state) override {
    // deterministically build an initial, fully-populated, order book
    book_ = std::make_unique<core::OrderBook>(bids_, asks_);

    // bids
    uint64_t bid_px = MID_PRICE - 1;
    for (uint64_t i = 0; i <= DEPTH_LEVELS; ++i) {
      auto msg = FIX44::MarketDataIncrementalRefresh();
      auto change = FIX44::MarketDataIncrementalRefresh::NoMDEntries();
      change.set(FIX::Symbol("BTCUSDT"));
      change.set(FIX::MDUpdateAction(FIX::MDUpdateAction_NEW));
      change.set(FIX::MDEntryType(FIX::MDEntryType_BID));
      change.set(FIX::MDEntryPx(bid_px--));
      if (bid_px < MID_PRICE - DEPTH_LEVELS) {
        bid_px = MID_PRICE - 1;
      }
      change.set(FIX::MDEntrySize(i));
      msg.addGroup(change);
      book_->apply_increment(msg, false);
    }

    // offers
    uint64_t ask_px = MID_PRICE + 1;
    for (uint64_t i = 0; i <= DEPTH_LEVELS; ++i) {
      auto msg = FIX44::MarketDataIncrementalRefresh();
      auto change = FIX44::MarketDataIncrementalRefresh::NoMDEntries();
      change.set(FIX::Symbol("BTCUSDT"));
      change.set(FIX::MDUpdateAction(FIX::MDUpdateAction_NEW));
      change.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
      change.set(FIX::MDEntryPx(ask_px++));
      if (ask_px > MID_PRICE + DEPTH_LEVELS) {
        ask_px = MID_PRICE + 1;
      }
      change.set(FIX::MDEntrySize(i));
      msg.addGroup(change);
      book_->apply_increment(msg, false);

      // asks_[MID_PRICE + i] = 1;
    }
  }

  void TearDown([[maybe_unused]] const benchmark::State& state) override {
    const auto vec = book_->to_vector();
    spdlog::info("Levels: [{}]", fmt::join(vec, ", "));
  }

  // order book
  std::unique_ptr<core::OrderBook> book_;
  absl::btree_map<uint64_t, uint64_t, std::greater<>> bids_;
  absl::btree_map<uint64_t, uint64_t> asks_;
  //
  static constexpr uint64_t DEPTH_LEVELS = 500;
  static constexpr uint64_t MID_PRICE = 100'000;
};

BENCHMARK_DEFINE_F(OrderBookFixture, BM_AddOrder)(benchmark::State& state) {
  FIX44::MarketDataIncrementalRefresh msg;
  FIX44::MarketDataIncrementalRefresh::NoMDEntries change;
  uint8_t tick_tock = 0;
  uint64_t volume = 1;
  uint64_t bid_px = MID_PRICE - 1;
  uint64_t ask_px = MID_PRICE + 1;
  for (auto _ : state) {
    msg = FIX44::MarketDataIncrementalRefresh();
    change = FIX44::MarketDataIncrementalRefresh::NoMDEntries();
    change.set(FIX::Symbol("BTCUSDT"));
    change.set(FIX::MDUpdateAction(FIX::MDUpdateAction_NEW));
    if (tick_tock == 0) {
      change.set(FIX::MDEntryType(FIX::MDEntryType_BID));
      change.set(FIX::MDEntryPx(bid_px--));
      if (bid_px < MID_PRICE - DEPTH_LEVELS) {
        bid_px = MID_PRICE - 1;
      }
      change.set(FIX::MDEntrySize(volume));
      tick_tock = 1;
    } else {
      change.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
      change.set(FIX::MDEntryPx(ask_px++));
      if (ask_px > MID_PRICE + DEPTH_LEVELS) {
        ask_px = MID_PRICE + 1;
      }
      change.set(FIX::MDEntrySize(volume++));
      tick_tock = 0;
    }
    msg.addGroup(change);
    book_->apply_increment(msg, false);
  }

  state.counters["Updates/sec"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}

BENCHMARK_REGISTER_F(OrderBookFixture, BM_AddOrder)->Iterations(500'000);
