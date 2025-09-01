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
		const char* fixCfgPath = std::getenv("FIX_CONFIG_PATH");
		std::cout << std::format("FIX_CONFIG_PATH, [{}]", fixCfgPath) <<  std::endl;
		MyApplication app;
		FIX::SessionSettings settings(fixCfgPath);
		FIX::FileStoreFactory storeFactory(settings);
		FIX::FileLogFactory logFactory(settings);
		FIX::SocketInitiator initiator(app, storeFactory, settings, logFactory);
		initiator.start();
		// while (true) { do something; }
		initiator.stop();
		return 0;
	} catch (FIX::ConfigError& e) {
		std::cout << e.what();
		return 1;
	}
}
