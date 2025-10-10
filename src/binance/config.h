#pragma once

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

namespace binance {

/// @brief Binance config parameters, fetched from env
struct Config {
 private:
  static constexpr uint32_t fnv1a_hash(std::string_view str) {
    uint32_t hash = 2166136261u;
    for (char c : str) {
      hash = (hash ^ c) * 16777619u;
    }
    return hash;
  }

 public:
  // TODO: MAKE UNIQUE POINTERS
  std::string api_key, private_key_path;
  const std::string fix_config_path;
  const std::vector<std::string> symbols;
  //
  static Config from_env();

  /// @brief fast runtime lookup
  /// @param symbol
  /// @return
  static constexpr int get_tick_size(std::string_view symbol) {
    switch (fnv1a_hash(symbol)) {
      case fnv1a_hash("BTCUSDT"):
        return 1;
      case fnv1a_hash("ETHUSDT"):
        return 1;
      case fnv1a_hash("DOGEUSDT"):
        return 1;
      default:
        throw std::runtime_error(
            std::format("cannot find tick size, symbol [{}]", symbol));
    }
  }

  /// @brief 1 == top level, otherwise 5000 is Binance's maximum depth
  static constexpr int MAX_DEPTH = 5000;
};

}  // namespace binance
