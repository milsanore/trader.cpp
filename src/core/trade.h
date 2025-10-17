#pragma once

#include <cmath>

#include "../binance/side.h"

using binance::Side;
using binance::SideEnum;

namespace core {

/// @brief Binance trade object
struct Trade {
 public:
  Trade(std::string time,
        const SideEnum side,
        const uint64_t px,
        const uint64_t sz,
        const uint64_t id)
      : time(std::move(time)), side(side), px(px), sz(sz), id(id) {}

  std::string time;
  SideEnum side;
  uint64_t px;
  uint64_t sz;
  uint64_t id;
};

}  // namespace core
