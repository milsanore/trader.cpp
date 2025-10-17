#pragma once

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>

namespace binance {

/// @brief An enum representation for symbol names, for high performance.
/// ensure `-Wswitch -Wswitch-enum` are set to catch missing handlers
enum class SymbolEnum : uint16_t {
  BTCUSDT = 0,
  ETHUSDT = 1,
};

/// @brief helper functions for @ref binance::SymbolEnum
struct Symbol {
  /// @brief convert a symbol string to an enum.
  /// uses an optimised perfect hash function (read, fragile).
  /// @param str
  /// @return
  static SymbolEnum from_str(const std::string& symbol) {
    if (symbol.size() != 7) {
      throw std::runtime_error(std::format(
          "cannot convert string to SymbolEnum - short string. value [{}]", symbol));
    }

    // ensure `-Wswitch -Wswitch-enum` are set to catch missing handlers
    switch (symbol[0]) {
      case 'B':
        return SymbolEnum::BTCUSDT;
      case 'E':
        return SymbolEnum::ETHUSDT;
    }

    throw std::runtime_error(
        std::format("cannot convert string to SymbolEnum - missing mapping. NB: this "
                    "should never throw. value [{}]",
                    symbol));
  }

  static std::string_view to_str_view(SymbolEnum symbol) {
    switch (symbol) {
      case SymbolEnum::BTCUSDT:
        return "BTCUSDT";
      case SymbolEnum::ETHUSDT:
        return "ETHUSDT";
    }
    throw std::runtime_error(std::format(
        "Unhandled SymbolEnum in to_str_view(). NB: this should never throw. value [{}]",
        to_uint(symbol)));
  }

  static std::string to_str(SymbolEnum symbol) {
    switch (symbol) {
      case SymbolEnum::BTCUSDT:
        return "BTCUSDT";
      case SymbolEnum::ETHUSDT:
        return "ETHUSDT";
    }
    throw std::runtime_error(std::format(
        "Unhandled SymbolEnum in to_str(). NB: this should never throw. value [{}]",
        to_uint(symbol)));
  }

  static constexpr uint16_t to_uint(const SymbolEnum m) noexcept {
    return static_cast<uint16_t>(m);
  }
};

}  // namespace binance
