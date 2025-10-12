#pragma once

#include <cmath>

namespace core {

/// @brief Binance trade object
struct Trade {
 public:
  Trade(std::string time, char side, double px, double sz, uint64_t id)
      : time(time), side(side), px(px), sz(sz), id(id) {}

  std::string time;
  char side;  // 1 == BUY, 2 == SELL
  double px;
  double sz;
  uint64_t id;
};

}  // namespace core
