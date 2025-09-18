#include <algorithm>
#include <cassert>
#include <fstream>
#include <format>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "Auth.h"
#include "FixApp.h"
#include <sodium.h>
#include <quickfix/FixValues.h>
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include "spdlog/spdlog.h"

namespace Binance {

// PRIVATE

/// Binance Message Handling Mode
/// Controls how the matching engine processes your messages
/// [More info](https://developers.binance.com/docs/binance-spot-api-docs/fix-api#on-message-processing-order)
enum class MessageHandlingMode : uint8_t {
	/// UNORDERED(1) - Messages from the client are allowed to be sent to the matching engine in any order
	Unordered = 1,
	/// SEQUENTIAL(2) - Messages from the client are always sent to the matching engine in MsgSeqNum (34) order
	Sequential = 2
};
std::string toString(const MessageHandlingMode m) {
	switch (m) {
		case MessageHandlingMode::Unordered: return "1";
		case MessageHandlingMode::Sequential: return "2";
		default: return "Unknown";
	}
}

// TODO: make debug only
std::string replaceSoh(const std::string& input) {
	std::string output = input;
	std::replace(output.begin(), output.end(), '\x01', '|');
	return output;
}

void FixApp::onCreate(const FIX::SessionID& sessionId) {
    spdlog::info(std::format("Session created, id [{}]", sessionId.toString()));
};
void FixApp::onLogon(const FIX::SessionID& sessionId) {
    spdlog::info(std::format("Session logon, id [{}]", sessionId.toString()));

	subscribeToDepth(sessionId);
};
void FixApp::onLogout(const FIX::SessionID& sessionId) {
    spdlog::info(std::format("Session logout, id [{}]", sessionId.toString()));

};
void FixApp::toAdmin(FIX::Message& msg, const FIX::SessionID& sessionId) {
	const FIX::Header& header = msg.getHeader();
	FIX::MsgType msgType;
	header.getField(msgType);
    spdlog::info(std::format("toAdmin, session Id [{}], type [{}], message [{}]",
		sessionId.toString(), msgType.getString(), replaceSoh(msg.toString())));
	
	if (msgType == FIX::MsgType_Logon) {
    	spdlog::info(std::format("authenticating"));
		
		// collect required fields
		const std::string sender = header.getField(FIX::FIELD::SenderCompID);
		const std::string target = header.getField(FIX::FIELD::TargetCompID);
		const std::string seqNum = msg.getHeader().getField(FIX::FIELD::MsgSeqNum);
		const auto sendingTime = FIX::UtcTimeStamp();

		// construct payload for signing
		const std::string payload = std::string("A")
									+ '\x01' + sender
									+ '\x01' + target
									+ '\x01' + seqNum
									+ '\x01' + FIX::UtcTimeStampConvertor::convert(sendingTime);
		const std::vector<unsigned char> seed = Binance::Auth::getSeedFromPem(privatePemPath_);
		const std::string signature = Binance::Auth::signPayload(payload, seed);
		assert(signature.size() <= INT_MAX);

		// assemble message
		msg.getHeader().setField(FIX::SendingTime(sendingTime));
		msg.setField(FIX::Username(apiKey_));
		msg.setField(FIX::RawData(signature));
		msg.setField(FIX::RawDataLength(static_cast<FIX::LENGTH>(signature.size())));
		msg.setField(FIX::StringField(25035, toString(MessageHandlingMode::Sequential)));
	}
};
void FixApp::toApp(FIX::Message& msg, const FIX::SessionID& sessionId) noexcept(false) {
	const FIX::Header& header = msg.getHeader();
	FIX::MsgType msgType;
	header.getField(msgType);
    spdlog::debug(std::format("toApp, session Id [{}], type [{}], message [{}]",
		sessionId.toString(), msgType.getString(), replaceSoh(msg.toString())));
};
void FixApp::fromAdmin(const FIX::Message& msg, const FIX::SessionID& sessionId) noexcept(false) {
	const FIX::Header& header = msg.getHeader();
	FIX::MsgType msgType;
	header.getField(msgType);
    spdlog::debug(std::format("fromAdmin, session Id [{}], type [{}], message [{}]",
		sessionId.toString(), msgType.getString(), replaceSoh(msg.toString())));
};
void FixApp::fromApp(const FIX::Message& msg, const FIX::SessionID& sessionId) noexcept(false) {
	FIX::MessageCracker::crack(msg, sessionId);
    spdlog::trace(std::format("fromApp, session Id [{}], message [{}]",
		sessionId.toString(), replaceSoh(msg.toString())));
}

void FixApp::onMessage(const FIX44::MarketDataSnapshotFullRefresh& m, const FIX::SessionID& sessionID) {
	auto msgPtr = std::make_shared<FIX44::MarketDataSnapshotFullRefresh>(m);
	queue.enqueue(msgPtr);
}
void FixApp::onMessage(const FIX44::MarketDataIncrementalRefresh& m, const FIX::SessionID& sessionID) {
	auto msgPtr = std::make_shared<FIX44::MarketDataIncrementalRefresh>(m);
	queue.enqueue(msgPtr);
}


// PUBLIC

FixApp::FixApp(const std::string& apiKey, const std::string& privatePemPath, const std::vector<std::string>& instruments) :
	// TODO: should I use references here?
	apiKey_(apiKey), privatePemPath_(privatePemPath), symbols_(instruments)
{
	if (sodium_init() < 0)
		throw std::runtime_error("libsodium failed to initialize");
}

void FixApp::subscribeToDepth(const FIX::SessionID& sessionId) {
    spdlog::debug(std::format("Subscribe to depth"));
	FIX44::MarketDataRequest marketDataRequest;

	// Generate a unique request ID for this session's request
	std::string reqId = "MDReq-" + std::to_string(std::time(nullptr));
	marketDataRequest.set(FIX::MDReqID(reqId));

	// Set subscription type (1 = Subscribe)
	marketDataRequest.set(FIX::SubscriptionRequestType(
		FIX::SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES));

	// Set market depth
	constexpr int BINANCE_MAX_DEPTH = 5000;
	marketDataRequest.set(FIX::MarketDepth(BINANCE_MAX_DEPTH));

	// Create NoMDEntryTypes group for requesting BID and OFFER
	FIX44::MarketDataRequest::NoMDEntryTypes entryTypeGroup;
	// Add BID entry type (0)
	entryTypeGroup.set(FIX::MDEntryType(FIX::MDEntryType_BID));
	marketDataRequest.addGroup(entryTypeGroup);
	// Add OFFER entry type (1)
	entryTypeGroup.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
	marketDataRequest.addGroup(entryTypeGroup);

	// Add symbols
	for (const auto& instrument : symbols_) {
		FIX44::MarketDataRequest::NoRelatedSym symbolGroup;
		symbolGroup.set(FIX::Symbol(instrument));
		marketDataRequest.addGroup(symbolGroup);
	}

	// Send the request to the corresponding market data session
	FIX::Session::sendToTarget(marketDataRequest, sessionId);
}

}
