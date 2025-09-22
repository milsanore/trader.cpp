#ifndef BINANCECONFIG_H
#define BINANCECONFIG_H

#include <string>
#include <vector>

namespace Binance {

/// @brief Binance config parameters, fetched from env
struct Config{
public:
    // TODO: MAKE UNIQUE POINTERS
    std::string apiKey, privateKeyPath;
    const std::string fixConfigPath;
    const std::vector<std::string> symbols;
    //
    static Config fromEnv();
};

}

#endif  // BINANCECONFIG_H
