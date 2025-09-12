#ifndef BINANCEFIXAPP_H
#define BINANCEFIXAPP_H

#include <quickfix/Application.h>
#include <quickfix/SessionID.h>
#include <quickfix/MessageCracker.h>
#include <map>

/// @brief Manages FIX connectivity to Binance
class BinanceFixApp final : public FIX::Application, public FIX::MessageCracker {
public:
	BinanceFixApp(std::string apiKey, std::string privatePemPath);
	~BinanceFixApp() override = default;
	void subscribeToDepth(const FIX::SessionID& sessionId);
	/// @brief print top ten order book levels
	void printBook();

private:
	std::string apiKey_, privatePemPath_;
	std::map<double, double> bidMap_;
	std::map<double, double> offerMap_;

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

#endif  // BINANCEFIXAPP_H
