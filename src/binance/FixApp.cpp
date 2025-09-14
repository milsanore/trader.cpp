#include <algorithm>
#include <cassert>
#include <fstream>
#include <format>
#include <iostream>
#include <iomanip>
#include <map>
#include <ranges>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include "Auth.h"
#include "FixApp.h"
#include <sodium.h>
#include <quickfix/FixValues.h>
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix44/MarketDataIncrementalRefresh.h>

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
	std::cout << std::format("Session created, id [{}]", sessionId.toString()) << std::endl;
};
void FixApp::onLogon(const FIX::SessionID& sessionId) {
	std::cout << std::format("Session logon, id [{}]", sessionId.toString()) << std::endl;
	subscribeToDepth(sessionId);
};
void FixApp::onLogout(const FIX::SessionID& sessionId) {
	std::cout << std::format("Session logout, id [{}]", sessionId.toString()) << std::endl;
};
void FixApp::toAdmin(FIX::Message& msg, const FIX::SessionID& sessionId) {
	const FIX::Header& header = msg.getHeader();
	FIX::MsgType msgType;
	header.getField(msgType);
	std::cout << std::format("toAdmin, session Id [{}], type [{}], message [{}]",
		sessionId.toString(), msgType.getString(), replaceSoh(msg.toString())) << std::endl;
	
	if (msgType == FIX::MsgType_Logon) {
		std::cout << std::format("authenticating") << std::endl;
		
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
	std::cout << std::format("toApp, session Id [{}], type [{}], message [{}]",
		sessionId.toString(), msgType.getString(), replaceSoh(msg.toString())) << std::endl;
};
void FixApp::fromAdmin(const FIX::Message& msg, const FIX::SessionID& sessionId) noexcept(false) {
	const FIX::Header& header = msg.getHeader();
	FIX::MsgType msgType;
	header.getField(msgType);
	std::cout << std::format("fromAdmin, session Id [{}], type [{}], message [{}]",
		sessionId.toString(), msgType.getString(), replaceSoh(msg.toString())) << std::endl;
};
void FixApp::fromApp(const FIX::Message& msg, const FIX::SessionID& sessionId) noexcept(false) {
	FIX::MessageCracker::crack(msg, sessionId);
	// std::cout << std::format("fromApp, session Id [{}], message [{}]",
	// 	sessionId.toString(), replaceSoh(msg.toString())) << std::endl;
}


void FixApp::printBook() {
	// Column headers
	std::cout << std::left
			<< std::setw(10) << "BID_SZ"
			<< std::setw(10) << "BID"
			<< std::setw(10) << "ASK"
			<< std::setw(10) << "ASK_SZ"
			<< "\n";
	// Divider
	std::cout << std::string(40, '-') << "\n";

	// print top 10x
	auto data = std::vector<std::tuple<double, double, double, double>>(10, std::make_tuple(0, 0, 0, 0));
	
	// bids
	int i = 0;
	for (auto&& [px, sz] : std::views::reverse(bidMap_)) {
		std::get<0>(data[i]) = sz;
		std::get<1>(data[i]) = px;
		i++;
		if (i > 9) 
			break;
	}

	// offers
	i = 0;
	for (const auto& [px, sz] : offerMap_) {
		std::get<2>(data[i]) = px;
		std::get<3>(data[i]) = sz;
		i++;
		if (i > 9) 
			break;
	}

	for (auto item: data) {
		std::cout << std::left
				<< std::setw(10) << std::get<0>(item)
				<< std::setw(10) << std::get<1>(item)
				<< std::setw(10) << std::get<2>(item)
				<< std::setw(10) << std::get<3>(item)
				<< "\n";
	}

	std::cout << std::flush;
}
void FixApp::onMessage(const FIX44::MarketDataSnapshotFullRefresh& m, const FIX::SessionID& sessionID) {
	FIX::Symbol symbol;
	m.get(symbol);
	std::cout << std::format("MD snapshot message, symbol [{}]", symbol.getString()) << std::endl;
	std::string symbolValue = symbol.getValue();
	if (symbolValue != "BTCUSDT") {
		std::cout << std::format("wrong symbol, skipping snapshot. value [{}]", symbolValue) << std::endl;
		return;
	}
	bidMap_.clear();
	offerMap_.clear();
	FIX::NoMDEntries noMDEntries;
	m.get(noMDEntries);
	int numEntries = noMDEntries.getValue();
	for (int i = 1; i <= numEntries; i++) {
		FIX44::MarketDataSnapshotFullRefresh::NoMDEntries group;
		m.getGroup(i, group);
		FIX::MDEntryType entryType;
		FIX::MDEntryPx px;
		FIX::MDEntrySize sz;
		group.get(entryType);
		group.get(px);
		group.get(sz);

		if (entryType == FIX::MDEntryType_BID) {
			bidMap_[px.getValue()] = sz.getValue();
		} else if (entryType == FIX::MDEntryType_OFFER) {
			offerMap_[px.getValue()] = sz.getValue();
		} else {
			std::cout << std::format("unknown bid/offer type [{}]", entryType.getString()) << std::endl;
		}
	}

	printBook();
}
void FixApp::onMessage(const FIX44::MarketDataIncrementalRefresh& message, const FIX::SessionID& sessionID) {
	FIX::NoMDEntries noMDEntries;
	message.get(noMDEntries);
	int numEntries = noMDEntries.getValue();
	for (int i = 1; i <= numEntries; i++) {
		FIX44::MarketDataIncrementalRefresh::NoMDEntries group;
		message.getGroup(i, group);

		FIX::Symbol symbol;
		if (group.isSetField(FIX::FIELD::Symbol)) {
			group.get(symbol);
		}
		// std::string symbolValue = symbol.getValue();
		// if (symbolValue != "BTCUSDT") {
		// 	std::cout << std::format("wrong symbol, skipping increment. value [{}]", symbolValue) << std::endl;
		// 	continue;
		// }
		
		FIX::MDUpdateAction action;
		FIX::MDEntryType entryType;
		FIX::MDEntryPx px;
		group.get(action);
		group.get(entryType);
		group.get(px);
		
		if (entryType != FIX::MDEntryType_BID && entryType != FIX::MDEntryType_OFFER) {
			std::cout << std::format("unknown entry type, skipping. value [{}]", entryType.getString()) << std::endl;
			continue;
		}

		if (action.getValue() == FIX::MDUpdateAction_NEW || action.getValue() == FIX::MDUpdateAction_CHANGE) {
			if (group.isSetField(FIX::FIELD::MDEntrySize)) {
				FIX::MDEntrySize sz;
				group.get(sz);
				if (entryType == FIX::MDEntryType_BID) {
					bidMap_[px.getValue()] = sz.getValue();
				} else if (entryType == FIX::MDEntryType_OFFER) {
					offerMap_[px.getValue()] = sz.getValue();
				}
			}
		} else if (action.getValue() == FIX::MDUpdateAction_DELETE) {
			if (entryType == FIX::MDEntryType_BID) {
				bidMap_.erase(px.getValue());
			} else if (entryType == FIX::MDEntryType_OFFER) {
				offerMap_.erase(px.getValue());
			}
		} else {
			std::cout << std::format("unknown action. value [{}]", action.getValue()) << std::endl;
		}
	}

	printBook();
}


// PUBLIC

FixApp::FixApp(std::string apiKey, std::string privatePemPath) : bidMap_{}, offerMap_{} {
	apiKey_ = std::move(apiKey);
	privatePemPath_ = std::move(privatePemPath);

	if (sodium_init() < 0)
		throw std::runtime_error("libsodium failed to initialize");
}

void FixApp::subscribeToDepth(const FIX::SessionID& sessionId) {
	std::cout << std::format("Subscribe to depth") << std::endl;

	FIX44::MarketDataRequest marketDataRequest;

	// Generate a unique request ID for this session's request
	std::string reqId = "MDReq-" + std::to_string(std::time(nullptr));
	marketDataRequest.set(FIX::MDReqID(reqId));

	// Set subscription type (1 = Subscribe)
	marketDataRequest.set(FIX::SubscriptionRequestType(
		FIX::SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES));

	// Set market depth
	marketDataRequest.set(FIX::MarketDepth(5)); // 1 = Top of book

	// Create NoMDEntryTypes group for requesting BID and OFFER
	FIX44::MarketDataRequest::NoMDEntryTypes entryTypeGroup;
	// Add BID entry type (0)
	entryTypeGroup.set(FIX::MDEntryType(FIX::MDEntryType_BID));
	marketDataRequest.addGroup(entryTypeGroup);
	// Add OFFER entry type (1)
	entryTypeGroup.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
	marketDataRequest.addGroup(entryTypeGroup);

	// Add symbol
	FIX44::MarketDataRequest::NoRelatedSym symbolGroup;
	symbolGroup.set(FIX::Symbol("BTCUSDT"));
	marketDataRequest.addGroup(symbolGroup);

	// Send the request to the corresponding market data session
	FIX::Session::sendToTarget(marketDataRequest, sessionId);
}

}
