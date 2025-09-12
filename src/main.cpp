#include <iostream>
#include <format>
#include "binance/BinanceInit.h"

int main() {
    std::cout << std::format("hello") << std::endl;
	auto bi = BinanceInit();
	std::cout << "Press any key to quit..." << std::endl;
    std::cin.get();
    std::cout << std::format("goodbye") << std::endl;
}
