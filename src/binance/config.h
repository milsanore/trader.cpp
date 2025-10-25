#pragma once

#include <cmath>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "../utils/double.h"
#include "symbol.h"

namespace binance {

/// @brief Binance config parameters, fetched from env
struct Config {
 public:
  // TODO: MAKE UNIQUE POINTERS
  std::string api_key, private_key_path;
  const std::string fix_config_path;
  const std::vector<std::string> symbols;
  //
  static Config from_env();

  /// @brief convert a market price to a tick representation, for performance and
  /// correctness
  static uint64_t price_to_ticks(const double price, const binance::SymbolEnum symbol) {
    return utils::Double::toUint64(price,
                                   binance::Config::get_price_ticks_per_unit(symbol));
  }

  /// @brief retrieves the equivalent of `1 / tick_size` for prices
  static constexpr uint64_t get_price_ticks_per_unit(const SymbolEnum s) {
    switch (s) {
      case SymbolEnum::BTCUSDT:
        return 100u;
      case SymbolEnum::ETHUSDT:
        return 100u;
    }
    throw std::runtime_error(
        std::format("cannot get price tick. NB: this should never throw. symbol [{}]",
                    Symbol::to_str(s)));
  }

  /// @brief retrieves the equivalent of `1 / tick_size` for volumes
  static constexpr uint64_t get_size_ticks_per_unit(const SymbolEnum s) {
    switch (s) {
      case SymbolEnum::BTCUSDT:
        return 100'000u;
      case SymbolEnum::ETHUSDT:
        return 10'000u;
    }
    throw std::runtime_error(
        std::format("cannot get size tick. NB: this should never throw. symbol [{}]",
                    Symbol::to_str(s)));
  }

  /// @brief 1 == top level, otherwise 5000 is Binance's maximum depth
  static constexpr uint16_t MAX_DEPTH = 100;

  static constexpr uint8_t PX_SESSION_CPU_AFFINITY = 0;
  static constexpr uint8_t TX_SESSION_CPU_AFFINITY = 1;
};

}  // namespace binance
