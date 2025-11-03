#include "binance/config.h"

#include <gtest/gtest.h>

#include <cstdlib>

using namespace binance;

TEST(ConfigTest, FromEnv) {
  // Setup test environment variables
  setenv("API_KEY", "test-api-key", 1);
  setenv("PRIVATE_KEY_PATH", "/path/to/private.key", 1);
  setenv("FIX_CONFIG_PATH", "/path/to/fix.cfg", 1);
  setenv("SYMBOLS", "BTCUSDT,ETHUSDT,BNBUSDT", 1);
  setenv("PX_SESSION_CPU", "0", 1);
  setenv("TX_SESSION_CPU", "1", 1);

  // Validate
  Config cfg = Config::from_env();
  EXPECT_EQ(cfg.api_key, "test-api-key");
  EXPECT_EQ(cfg.private_key_path, "/path/to/private.key");
  EXPECT_EQ(cfg.fix_config_path, "/path/to/fix.cfg");
  std::vector<std::string> expectedSymbols = {"BTCUSDT", "ETHUSDT", "BNBUSDT"};
  EXPECT_EQ(cfg.symbols, expectedSymbols);
  EXPECT_EQ(cfg.px_cpu, 0);
  EXPECT_EQ(cfg.tx_cpu, 1);
}

TEST(ConfigTest, ThrowsOnMissingEnv) {
  unsetenv("API_KEY");
  setenv("PRIVATE_KEY_PATH", "somekey", 1);
  setenv("FIX_CONFIG_PATH", "somecfg", 1);
  setenv("SYMBOLS", "BTCUSDT", 1);
  setenv("PX_SESSION_CPU", "0", 1);
  setenv("TX_SESSION_CPU", "1", 1);

  EXPECT_THROW(Config::from_env(), std::runtime_error);
}

TEST(ConfigTest, HandlesEmptySymbols) {
  setenv("API_KEY", "key", 1);
  setenv("PRIVATE_KEY_PATH", "keypath", 1);
  setenv("FIX_CONFIG_PATH", "fix", 1);
  setenv("SYMBOLS", "", 1);
  setenv("PX_SESSION_CPU", "0", 1);
  setenv("TX_SESSION_CPU", "1", 1);

  EXPECT_THROW(Config::from_env(), std::runtime_error);
}

TEST(ConfigTest, HandlesTrailingComma) {
  setenv("API_KEY", "key", 1);
  setenv("PRIVATE_KEY_PATH", "keypath", 1);
  setenv("FIX_CONFIG_PATH", "fix", 1);
  setenv("SYMBOLS", "BTCUSDT,", 1);
  setenv("PX_SESSION_CPU", "0", 1);
  setenv("TX_SESSION_CPU", "1", 1);

  Config cfg = Config::from_env();
  std::vector<std::string> expected = {"BTCUSDT"};
  EXPECT_EQ(cfg.symbols, expected);
}
