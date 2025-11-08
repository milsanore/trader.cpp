#include <benchmark/benchmark.h>
#include <fmt/ranges.h>

#include <random>

#include "core/bid_ask.h"
#include "core/order_book.h"
#include "spdlog/spdlog.h"

class PriceUpdateFixture : public benchmark::Fixture {
 public:
  /// @brief deterministically build an initial, fully-populated, order book
  void SetUp([[maybe_unused]] const benchmark::State& state) override {
    book_ = std::make_unique<core::OrderBook>(bids_, asks_);

    // bids
    uint64_t bid_px = MID_PRICE - 1;
    for (uint64_t i = 0; i < DEPTH_LEVELS; ++i) {
      auto msg = FIX44::MarketDataIncrementalRefresh();
      auto change = FIX44::MarketDataIncrementalRefresh::NoMDEntries();
      change.set(FIX::Symbol("BTCUSDT"));
      change.set(FIX::MDUpdateAction(FIX::MDUpdateAction_NEW));
      change.set(FIX::MDEntryType(FIX::MDEntryType_BID));
      change.set(FIX::MDEntryPx(bid_px--));
      if (bid_px < MID_PRICE - DEPTH_LEVELS) {
        bid_px = MID_PRICE - 1;
      }
      change.set(FIX::MDEntrySize(1));
      msg.addGroup(change);
      book_->apply_increment(msg, false);
    }

    // offers
    uint64_t ask_px = MID_PRICE + 1;
    for (uint64_t i = 0; i < DEPTH_LEVELS; ++i) {
      auto msg = FIX44::MarketDataIncrementalRefresh();
      auto change = FIX44::MarketDataIncrementalRefresh::NoMDEntries();
      change.set(FIX::Symbol("BTCUSDT"));
      change.set(FIX::MDUpdateAction(FIX::MDUpdateAction_NEW));
      change.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
      change.set(FIX::MDEntryPx(ask_px++));
      if (ask_px > MID_PRICE + DEPTH_LEVELS) {
        ask_px = MID_PRICE + 1;
      }
      change.set(FIX::MDEntrySize(1));
      msg.addGroup(change);
      book_->apply_increment(msg, false);

      // asks_[MID_PRICE + i] = 1;
    }

    // test messages
    uint8_t tick_tock = 0;
    uint64_t volume = 1;
    bid_px = MID_PRICE - 1;
    ask_px = MID_PRICE + 1;
    for (int i = 0; i < MSG_COUNT; i++) {
      auto msg = FIX44::MarketDataIncrementalRefresh();
      auto change = FIX44::MarketDataIncrementalRefresh::NoMDEntries();
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
      test_messages_[i] = msg;
    }
  }

  void TearDown([[maybe_unused]] const benchmark::State& state) override {
    // const auto vec = book_->to_vector();
    // spdlog::info("Levels: [{}]", fmt::join(vec, ", "));
  }

  // order book
  std::unique_ptr<core::OrderBook> book_;
  absl::btree_map<uint64_t, uint64_t, std::greater<>> bids_;
  absl::btree_map<uint64_t, uint64_t> asks_;
  static constexpr uint64_t DEPTH_LEVELS = 500;
  static constexpr uint64_t MID_PRICE = 100'000;
  // test messages
  static constexpr int MSG_COUNT = 1000;
  std::array<FIX44::MarketDataIncrementalRefresh, 1000> test_messages_;
};

/// @brief deterministically populate the order book
BENCHMARK_DEFINE_F(PriceUpdateFixture, BM_PriceUpdate)(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    book_->apply_increment(test_messages_[i], false);
    if (i == MSG_COUNT - 1) {
      i = 0;
    }
  }

  state.counters["Updates/sec"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}

BENCHMARK_REGISTER_F(PriceUpdateFixture, BM_PriceUpdate)->Iterations(500'000);
