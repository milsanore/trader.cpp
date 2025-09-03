#include <iostream>
#include <format>
#include <quickfix/Application.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include "MyApplication.h"

int main() {
	try {
		std::cout << "Hello, world!" << std::endl;
		
		const char* apiKey 	= std::getenv("API_KEY");
		const char* fixCfgPath = std::getenv("FIX_CONFIG_PATH");
		const char* privKeyPath = std::getenv("PRIVATE_KEY_PATH");

		std::cout << std::format("API_KEY, [{}]", apiKey) << std::endl;
		std::cout << std::format("FIX_CONFIG_PATH, [{}]", fixCfgPath) << std::endl;
		std::cout << std::format("PRIVATE_KEY_PATH, [{}]", privKeyPath) << std::endl;

		MyApplication app(apiKey, privKeyPath);
		FIX::SessionSettings settings(fixCfgPath);
		FIX::FileStoreFactory storeFactory(settings);
		FIX::FileLogFactory logFactory(settings);
		FIX::SocketInitiator initiator(app, storeFactory, settings, logFactory);
		initiator.start();
	    std::cout << "Press Enter to quit..." << std::endl;
		std::cin.get();
		initiator.stop();
	} catch (FIX::ConfigError& e) {
		std::cout << e.what();
		return 1;
	}
}
