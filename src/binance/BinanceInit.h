#ifndef BINANCEINIT_H
#define BINANCEINIT_H

#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include "BinanceFixApp.h"

class BinanceInit final {
public:
    BinanceInit();
	~BinanceInit();

private:
    BinanceFixApp app_;
    FIX::SessionSettings settings_;
    FIX::FileStoreFactory storeFactory_;
    FIX::FileLogFactory logFactory_;
    FIX::SocketInitiator initiator_;
};

#endif  // BINANCEINIT_H
