#include "Config.h"

#include <format>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "spdlog/spdlog.h"

namespace Binance {

/// @brief load a variable from the environment, if it's not available, panic
/// @param key the name of the environment variable
/// @return env var string
std::string getEnvOrThrow(const char *key) {
  if (const char *val = std::getenv(key)) {
    spdlog::info(
        std::format("fetched environment variable, key [{}], value [{}]", key, val));
    return std::string(val);
  }
  throw std::runtime_error(std::format("envvar not defined, key [{}]", key));
};

// TODO: for keys, use `std::vector<unsigned char>` instead of string

/// @brief load Binance configuration from env
/// (static member function)
/// @return Config object
Config Config::fromEnv() {
  const std::string apiKey = getEnvOrThrow("API_KEY");
  const std::string privateKey = getEnvOrThrow("PRIVATE_KEY_PATH");
  const std::string fixConfig = getEnvOrThrow("FIX_CONFIG_PATH");
  const std::string instStr = getEnvOrThrow("SYMBOLS");
  std::vector<std::string> symbols;
  for (auto inst : std::views::split(instStr, ',')) {
    if (inst.size() > 0) symbols.emplace_back(inst.begin(), inst.end());
  }

  // copy
  return Config{apiKey, privateKey, fixConfig, symbols};
};

}  // namespace Binance
