#pragma once

#include <fmt/core.h>

#include "../utils/env.h"

namespace core {

/// @brief basic bid/ask level
struct alignas(utils::Env::CACHE_LINE_SIZE) BidAsk {
 public:
  BidAsk() = default;
  explicit BidAsk(u_int64_t bidsz, u_int64_t bidpx, u_int64_t askpx, u_int64_t asksz)
      : bid_sz(bidsz), bid_px(bidpx), ask_px(askpx), ask_sz(asksz) {}

  // NB: using UINT64_MAX as a sentinel value.
  // maybe it's smarter to use UINT64_MAX-1 as a sentinel,
  // in order to catch overflow prices in the market?
  static inline constexpr u_int64_t SENTINEL_ = UINT64_MAX;

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

// BidAsk formatter functions
namespace fmt {
template <>
struct formatter<core::BidAsk> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const core::BidAsk& ba, FormatContext& ctx) const {
    return format_to(ctx.out(), "{{bid_sz: {}, bid_px: {}, ask_px: {}, ask_sz: {}}}",
                     ba.bid_sz, ba.bid_px, ba.ask_px, ba.ask_sz);
  }
};
}  // namespace fmt
