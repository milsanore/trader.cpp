#pragma once

#include <cmath>

namespace core {

/// @brief basic bid/ask level
struct BidAsk {
 public:
  BidAsk() = default;
  explicit BidAsk(u_int64_t bidsz, u_int64_t bidpx, u_int64_t askpx, u_int64_t asksz)
      : bid_sz(bidsz), bid_px(bidpx), ask_px(askpx), ask_sz(asksz) {}

  // NB: using UINT64_MAX as a sentinel value.
  // maybe it's smarter to use UINT64_MAX-1 as a sentinel,
  // in order to catch overflow prices in the market?
  static constexpr u_int64_t SENTINEL_ = UINT64_MAX;

  u_int64_t bid_sz = SENTINEL_;
  u_int64_t bid_px = SENTINEL_;
  u_int64_t ask_px = SENTINEL_;
  u_int64_t ask_sz = SENTINEL_;

  bool operator==(const BidAsk& other) const {
    return bid_sz == other.bid_sz && bid_px == other.bid_px && ask_sz == other.ask_sz &&
           ask_px == other.ask_px;
  }
};

}  // namespace core
