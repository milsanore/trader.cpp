#ifndef BINANCECONFIG_H
#define BINANCECONFIG_H

#include <string>

namespace Binance {

/// @brief Binance config parameters, fetched from env
struct Config{
public:
    const std::string apiKey;
    const std::string privateKeyPath;
    const std::string fixConfigPath;

    static Config fromEnv();
};

}

#endif  // BINANCECONFIG_H
