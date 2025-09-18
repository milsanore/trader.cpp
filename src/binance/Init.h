#ifndef BINANCEINIT_H
#define BINANCEINIT_H

#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include "FixApp.h"
#include "Config.h"

namespace Binance {

/// @brief Binance DI container
class Init final {
public:
    Init(std::unique_ptr<FixApp> fixApp,
        std::unique_ptr<FIX::FileStoreFactory> fileStoreFactory,
        std::unique_ptr<FIX::SessionSettings> settings,
        std::unique_ptr<FIX::FileLogFactory> fileLogFactory);
	~Init();
    static Init fromConf(Config& conf);
    void start();
    void stop();
    std::unique_ptr<FixApp> app;

private:
    // TODO: rename
    std::unique_ptr<FIX::FileStoreFactory> store_;
    std::unique_ptr<FIX::SessionSettings> settings_;
    std::unique_ptr<FIX::FileLogFactory> log_;
    FIX::SocketInitiator initiator_;
};

}

#endif  // BINANCEINIT_H
