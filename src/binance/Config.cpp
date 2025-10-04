#include "Config.h"

#include <format>
#include <ranges>
#include <string>
#include <vector>

#include "../core/Env.h"
#include "spdlog/spdlog.h"

namespace binance {
// TODO(mils): for keys, use `std::vector<unsigned char>` instead of string

/// @brief load Binance configuration from env
/// (static member function)
/// @return Config object
Config Config::fromEnv() {
  const std::string apiKey = core::Env::getEnvOrThrow("API_KEY");
  const std::string privateKey = core::Env::getEnvOrThrow("PRIVATE_KEY_PATH");
  const std::string fixConfig = core::Env::getEnvOrThrow("FIX_CONFIG_PATH");
  const std::string instStr = core::Env::getEnvOrThrow("SYMBOLS");

  spdlog::info("fetched environment variable, key [API_KEY], value [{}]",
               apiKey.substr(0, 3) + "..." + apiKey.substr(apiKey.length() - 3));
  spdlog::info("fetched environment variable, key [PRIVATE_KEY_PATH], value [{}]",
               privateKey);
  spdlog::info("fetched environment variable, key [FIX_CONFIG_PATH], value [{}]",
               fixConfig);
  spdlog::info("fetched environment variable, key [SYMBOLS], value [{}]", instStr);

  std::vector<std::string> symbols;
  for (auto inst : std::views::split(instStr, ',')) {
    if (!inst.empty()) {
      symbols.emplace_back(inst.begin(), inst.end());
    }
  }

  // copy
  return Config{apiKey, privateKey, fixConfig, symbols};
};

}  // namespace binance
