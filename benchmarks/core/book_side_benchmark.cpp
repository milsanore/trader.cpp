#include <benchmark/benchmark.h>
#include <fmt/ranges.h>

#include "core/order_book.h"
#include "spdlog/spdlog.h"

/// @brief the order book is composed of two asymmetrically-sorted collections,
/// this test is for one of those collections,
/// (the bid side in this case, because of the additional complexity of a DESC sort)
class BookSideFixture : public benchmark::Fixture {
 public:
  /// @brief deterministically build an initial, fully-populated, bid-side of the book
  void SetUp([[maybe_unused]] const benchmark::State& state) override {
    // bids
    for (uint64_t i = 0; i < DEPTH_LEVELS; ++i) {
      bids_[MID_PRICE - i] = 1;
    }
  }

  void TearDown([[maybe_unused]] const benchmark::State& state) override {
    // spdlog::info("Levels: [{}]", bids_);
  }

  absl::btree_map<uint64_t, uint64_t, std::greater<>> bids_;
  static constexpr uint64_t DEPTH_LEVELS = 500;
  static constexpr uint64_t MID_PRICE = 100'000;
};

/// @brief deterministically populate the order book
BENCHMARK_DEFINE_F(BookSideFixture, BENCH_BookUpdate)(benchmark::State& state) {
  int i = 0;
  for (auto _ : state) {
    bids_[i] = MID_PRICE - i;
    if (i == MID_PRICE - 1) {
      i = 0;
    }
    i++;
  }

  state.counters["Updates/sec"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}

BENCHMARK_REGISTER_F(BookSideFixture, BENCH_BookUpdate)->Iterations(500'000);
