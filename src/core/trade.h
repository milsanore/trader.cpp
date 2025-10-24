#pragma once

#include <cmath>

#include "../binance/side.h"
#include "../utils/env.h"

using binance::Side;
using binance::SideEnum;

namespace core {

/// @brief Binance trade object
struct alignas(utils::Env::CACHE_LINE_SIZE) Trade {
 public:
  Trade(const uint64_t px,
        const uint64_t sz,
        const uint64_t id,
        const SideEnum side,
        std::array<char, 16> time)
      : px(px), sz(sz), id(id), side(side), time(time) {}

  uint64_t px;
  uint64_t sz;
  uint64_t id;
  SideEnum side;
  // 15 characters + newline, e.g. "07:17:50.031794"
  std::array<char, 16> time;
};

}  // namespace core
