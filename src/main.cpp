#include <iostream>
#include <format>
#include <quickfix/Application.h>
#include <quickfix/SessionSettings.h>

int main() {
	std::cout << "Hello, world!" << std::endl;

	const char* fixCfgPath = std::getenv("FIX_CONFIG_PATH");
	std::cout << std::format("FIX_CONFIG_PATH, [{}]", fixCfgPath) <<  std::endl;
	FIX::SessionSettings settings(fixCfgPath);

	return 0;
}
