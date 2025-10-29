#pragma once

#include <fix8_router.hpp>
#include <functional>

#include "includes.h"
#include "session.h"

namespace f8 {

class Session;

class Router : public FIX8::binance::fix8_Router {
  [[maybe_unused]] Session& _session;

 public:
  explicit Router(Session& session) : _session(session) {}
  ~Router() override = default;

  // Override these methods to receive specific message callbacks.
  // bool operator() (const FIX8::binance::Heartbeat *msg) const;
  // bool operator() (const FIX8::binance::TestRequest *msg) const;
  // bool operator() (const FIX8::binance::Reject *msg) const;
  // bool operator() (const FIX8::binance::Logout *msg) const;
  // bool operator() (const FIX8::binance::Logon *msg) const;
  // bool operator() (const FIX8::binance::News *msg) const;
  // bool operator() (const FIX8::binance::MarketDataRequest *msg) const;
  // bool operator() (const FIX8::binance::MarketDataSnapshot *msg) const;
  // bool operator() (const FIX8::binance::MarketDataIncrementalRefresh *msg) const;
  // bool operator() (const FIX8::binance::LimitQuery *msg) const;
  // bool operator() (const FIX8::binance::LimitResponse *msg) const;
  // bool operator() (const FIX8::binance::MarketDataRequestReject *msg) const;
  // bool operator() (const FIX8::binance::InstrumentListRequest *msg) const;
  // bool operator() (const FIX8::binance::InstrumentList *msg) const;
};

}  // namespace f8
