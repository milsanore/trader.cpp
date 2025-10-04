#pragma once

#include <string>
#include <vector>

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
};

}  // namespace binance
