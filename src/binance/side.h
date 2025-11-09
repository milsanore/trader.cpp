#pragma once

#include <format>
#include <stdexcept>
#include <string>

namespace binance {

enum class SideEnum : char {
  BUY = '1',
  SELL = '2',
};

/// @brief helper functions for @ref binance::SideEnum
struct Side {
  /// @brief convert a Binance `AggressorSide` char to an enum.
  static SideEnum from_str(const char side) {
    // ensure `-Wswitch -Wswitch-enum` are set to catch missing handlers
    switch (side) {
      case '1':
        return SideEnum::BUY;
      case '2':
        return SideEnum::SELL;
    }

    throw std::runtime_error(
        std::format("cannot convert string to SideEnum - missing mapping. NB: this "
                    "should never throw. value [{}]",
                    side));
  }

  static std::string_view to_str_view(SideEnum side) {
    switch (side) {
      case SideEnum::BUY:
        return "BUY";
      case SideEnum::SELL:
        return "SELL";
    }
    throw std::runtime_error(std::format(
        "Unhandled SideEnum in to_str_view(). NB: this should never throw. value [{}]",
        to_char(side)));
  }

  static std::string to_str(SideEnum side) {
    switch (side) {
      case SideEnum::BUY:
        return "BUY";
      case SideEnum::SELL:
        return "SELL";
    }
    throw std::runtime_error(std::format(
        "Unhandled SideEnum in to_str(). NB: this should never throw. value [{}]",
        to_char(side)));
  }

  static constexpr char to_char(const SideEnum m) noexcept {
    return static_cast<char>(m);
  }
};

}  // namespace binance
