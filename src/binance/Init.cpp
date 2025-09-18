#include <memory>
#include <quickfix/Application.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include "Init.h"
#include "FixApp.h"
#include "Config.h"

namespace Binance {

Init::Init(std::unique_ptr<FixApp> fixApp,
            std::unique_ptr<FIX::FileStoreFactory> fileStoreFactory,
            std::unique_ptr<FIX::SessionSettings> settings,
            std::unique_ptr<FIX::FileLogFactory> fileLogFactory)
    : app(std::move(fixApp)),
        store_(std::move(fileStoreFactory)),
        settings_(std::move(settings)),
        log_(std::move(fileLogFactory)),
        initiator_(*app, *store_, *settings_, *log_) {}

// static member function
Init Init::fromConf(Config& conf) {
    auto app            = std::make_unique<FixApp>(conf.apiKey, conf.privateKeyPath);
    auto settings       = std::make_unique<FIX::SessionSettings>(conf.fixConfigPath);
    auto storeFactory   = std::make_unique<FIX::FileStoreFactory>(*settings);
    auto logFactory     = std::make_unique<FIX::FileLogFactory>(*settings);

    // hand over object ownership to the instance being created (by the static function)

    // TODO: is this being moved twice, because of the std::move() calls in the initializer list?
    return {std::move(app),
            std::move(storeFactory),
            std::move(settings),
            std::move(logFactory) };
}

void Init::start() {
    initiator_.start();
    std::cout << "started FIX session" << std::endl;
}

void Init::stop() {
    try {
        initiator_.stop(); // TODO: does this need a try/catch?
        std::cout << "stopped FIX session" << std::endl;
    } catch (...) {
        std::cout << "error closing binance FIX session" << std::endl;
    }
}

Init::~Init()
{
    initiator_.stop();
}

}
