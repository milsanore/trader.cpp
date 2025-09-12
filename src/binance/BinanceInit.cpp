#include <format>
#include <quickfix/Application.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include "BinanceInit.h"
#include "BinanceFixApp.h"

BinanceInit::BinanceInit() :
    app_(std::getenv("API_KEY"), std::getenv("PRIVATE_KEY_PATH")),
    settings_(std::getenv("FIX_CONFIG_PATH")),
    storeFactory_(settings_),
    logFactory_(settings_),
    initiator_(app_, storeFactory_, settings_, logFactory_)
{
    const char* apiKey 	        = std::getenv("API_KEY");
    const char* fixCfgPath      = std::getenv("FIX_CONFIG_PATH");
    const char* privatePemPath  = std::getenv("PRIVATE_KEY_PATH");

    std::cout << std::format("API_KEY, [{}]", apiKey) << std::endl;
    std::cout << std::format("FIX_CONFIG_PATH, [{}]", fixCfgPath) << std::endl;
    std::cout << std::format("PRIVATE_PEM_PATH, [{}]", privatePemPath) << std::endl;

    initiator_.start();
}

BinanceInit::~BinanceInit() {
    initiator_.stop(); // TODO: does this need a try/catch?
}
