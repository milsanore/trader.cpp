#pragma once

#include <cmath>

namespace core {

/// @brief Binance trade object
struct Trade {
 public:
  Trade(std::string time,
        const char side,
        const double px,
        const double sz,
        const uint64_t id)
      : time(std::move(time)), side(side), px(px), sz(sz), id(id) {}

  std::string time;
  /// side, 1 == BUY, 2 == SELL
  char side;
  double px;
  double sz;
  uint64_t id;
};

}  // namespace core
