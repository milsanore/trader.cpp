#include <format>
#include <iostream>
#include <stdexcept>
#include <string>
#include "Config.h"

namespace Binance {

Config Config::fromEnv() {
    auto getEnvOrThrow = [](const char* key) -> std::string {
        if (const char* val = std::getenv(key)) {
            std::cout << std::format("fetched environment variable, key [{}], value [{}]", key, val) << std::endl;
            return std::string(val);
        }
        throw std::runtime_error(std::format("envvar not defined, key [{}]", key));
    };

    std::string apiKey      = getEnvOrThrow("API_KEY");
    std::string privateKey  = getEnvOrThrow("PRIVATE_KEY_PATH");
    std::string fixConfig   = getEnvOrThrow("FIX_CONFIG_PATH");

    return Config{
        apiKey,
        privateKey,
        fixConfig
    };
};

}