#ifndef BINANCEFIXAPP_H
#define BINANCEFIXAPP_H

#include <quickfix/Application.h>
#include <quickfix/SessionID.h>
#include <quickfix/fix44/Message.h>
#include <quickfix/MessageCracker.h>
#include "concurrentqueue.h"

namespace Binance {

/// @brief Binance FIX App - Manages FIX connectivity to Binance
class FixApp final : public FIX::Application, public FIX::MessageCracker {
public:
	FixApp(std::string apiKey, std::string privatePemPath);
	~FixApp() override = default;
	void subscribeToDepth(const FIX::SessionID& sessionId);

	// TODO: performance implication of a polymorphic queue
	// TODO: perhaps better to run two queues, or something else
	// TODO: we will have one of these per instrument
	moodycamel::ConcurrentQueue<std::shared_ptr<FIX44::Message>> queue;

private:
	// TODO: no need to persist access tokens for lifetime of app
	std::string apiKey_, privatePemPath_;

	void onCreate(const FIX::SessionID&) override;
	void onLogon(const FIX::SessionID&) override;
	void onLogout(const FIX::SessionID&) override;
	void toAdmin(FIX::Message&, const FIX::SessionID&) override;
	void toApp(FIX::Message&, const FIX::SessionID&) noexcept(false) override;
	void fromAdmin(const FIX::Message&, const FIX::SessionID&) noexcept(false) override;
	void fromApp(const FIX::Message&, const FIX::SessionID&) noexcept(false) override;
	
	// Callbacks for specific message types / MessageCracker overloads
	void onMessage(const FIX44::MarketDataSnapshotFullRefresh &, const FIX::SessionID &) override;
	void onMessage(const FIX44::MarketDataIncrementalRefresh &, const FIX::SessionID &) override;
};

}

#endif  // BINANCEFIXAPP_H
