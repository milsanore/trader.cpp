#include <iostream>
#include <format>
#include "binance/Config.h"
#include "binance/Init.h"

int main() {
    std::cout << std::format("hello") << std::endl;

    auto bConf = Binance::Config::fromEnv();
	auto binance = Binance::Init::fromConf(bConf);
    binance.start();

    std::cout << "Press any key to quit..." << std::endl;
    std::cin.get();

    binance.stop();

    std::cout << std::format("goodbye") << std::endl;
}
