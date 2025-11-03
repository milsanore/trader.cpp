#include "config.h"

#include <format>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "../utils/env.h"
#include "spdlog/spdlog.h"

namespace binance {
// TODO(mils): for keys, use `std::vector<unsigned char>` instead of string

// static function
Config Config::from_env() {
  const std::string api_key = utils::Env::get_env_or_throw("API_KEY");
  const std::string private_key = utils::Env::get_env_or_throw("PRIVATE_KEY_PATH");
  const std::string fix_config = utils::Env::get_env_or_throw("FIX_CONFIG_PATH");
  const std::string px_cpu_str = utils::Env::get_env_or_throw("PX_SESSION_CPU");
  const std::string tx_cpu_str = utils::Env::get_env_or_throw("TX_SESSION_CPU");
  const std::string inst_str = utils::Env::get_env_or_throw("SYMBOLS");

  uint8_t px_cpu;
  std::errc px_ec =
      std::from_chars(px_cpu_str.data(), px_cpu_str.data() + px_cpu_str.size(), px_cpu)
          .ec;
  if (px_ec != std::errc()) {
    throw std::runtime_error(
        std::format("could not parse px cpu, value [{}]", px_cpu_str));
  }

  uint8_t tx_cpu;
  std::errc tx_ec =
      std::from_chars(tx_cpu_str.data(), tx_cpu_str.data() + tx_cpu_str.size(), tx_cpu)
          .ec;
  if (tx_ec != std::errc()) {
    throw std::runtime_error(
        std::format("could not parse tx cpu, value [{}]", tx_cpu_str));
  }

  spdlog::info("fetched envar. key [API_KEY], value [{}]",
               api_key.substr(0, 3) + "..." + api_key.substr(api_key.length() - 3));
  spdlog::info("fetched envar. key [PRIVATE_KEY_PATH], value [{}]", private_key);
  spdlog::info("fetched envar. key [FIX_CONFIG_PATH], value [{}]", fix_config);
  spdlog::info("fetched envar. key [SYMBOLS], value [{}]", inst_str);
  spdlog::info("fetched envar. key [PX_SESSION_CPU], value [{}]", px_cpu_str);
  spdlog::info("fetched envar. key [TX_SESSION_CPU], value [{}]", tx_cpu_str);

  std::vector<std::string> symbols;
  for (auto inst : std::views::split(inst_str, ',')) {
    if (!inst.empty()) {
      symbols.emplace_back(inst.begin(), inst.end());
    }
  }

  spdlog::info("MAX_DEPTH, value [{}]", MAX_DEPTH);

  // copy
  return Config{api_key, private_key, fix_config, symbols, px_cpu, tx_cpu};
};

}  // namespace binance
