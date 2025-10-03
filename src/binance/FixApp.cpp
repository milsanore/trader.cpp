#include "FixApp.h"

#include <quickfix/FixValues.h>
#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <sodium.h>

#include <algorithm>
#include <cassert>
#include <format>
#include <memory>
#include <string>
#include <vector>

#include "MessageHandlingMode.h"
#include "spdlog/spdlog.h"

namespace Binance {

// PRIVATE

// better FIX message logging
std::string replaceSoh(const std::string &input) {
  std::string output = input;
  std::ranges::replace(output.begin(), output.end(), '\x01', '|');
  return output;
}

void FixApp::onCreate(const FIX::SessionID &sessionId) {
  spdlog::info("Session created, id [{}]", sessionId.toString());
};
void FixApp::onLogon(const FIX::SessionID &sessionId) {
  spdlog::info("Session logon, id [{}]", sessionId.toString());
  // logon successful, nullify access keys
  auth_->clearKeys();
  subscribeToDepth(sessionId);
};
void FixApp::onLogout(const FIX::SessionID &sessionId) {
  spdlog::info("Session logout, id [{}]", sessionId.toString());
};
void FixApp::toAdmin(FIX::Message &msg, const FIX::SessionID &sessionId) {
  const FIX::Header &header = msg.getHeader();
  FIX::MsgType msgType;
  header.getField(msgType);
  spdlog::info("toAdmin, session Id [{}], type [{}], message [{}]", sessionId.toString(),
               msgType.getString(), replaceSoh(msg.toString()));

  if (msgType.getString() == static_cast<const char *>(FIX::MsgType_Logon)) {
    spdlog::info("authenticating");

    // collect required fields
    const std::string sender = header.getField(FIX::FIELD::SenderCompID);
    const std::string target = header.getField(FIX::FIELD::TargetCompID);
    const std::string seqNum = msg.getHeader().getField(FIX::FIELD::MsgSeqNum);
    const auto sendingTime = FIX::UtcTimeStamp();

    // construct payload for signing
    const std::string payload = std::string("A") + '\x01' + sender + '\x01' + target +
                                '\x01' + seqNum + '\x01' +
                                FIX::UtcTimeStampConvertor::convert(sendingTime);
    const std::string signature = auth_->signPayload(payload);
    assert(signature.size() <= INT_MAX);

    // assemble message
    msg.getHeader().setField(FIX::SendingTime(sendingTime));
    msg.setField(FIX::Username(auth_->getApiKey()));
    msg.setField(FIX::RawData(signature));
    msg.setField(FIX::RawDataLength(static_cast<FIX::LENGTH>(signature.size())));
    msg.setField(FIX::StringField(toInt(MessageHandlingMode::FIELD_ID),
                                  toString(MessageHandlingMode::Sequential)));
  }
};
void FixApp::toApp(FIX::Message &msg, const FIX::SessionID &sessionId) noexcept(false) {
  const FIX::Header &header = msg.getHeader();
  FIX::MsgType msgType;
  header.getField(msgType);
  spdlog::debug("toApp, session Id [{}], type [{}], message [{}]", sessionId.toString(),
                msgType.getString(), replaceSoh(msg.toString()));
};
void FixApp::fromAdmin(const FIX::Message &msg,
                       const FIX::SessionID &sessionId) noexcept(false) {
  const FIX::Header &header = msg.getHeader();
  FIX::MsgType msgType;
  header.getField(msgType);
  spdlog::debug("fromAdmin, session Id [{}], type [{}], message [{}]",
                sessionId.toString(), msgType.getString(), replaceSoh(msg.toString()));
};
void FixApp::fromApp(const FIX::Message &msg,
                     const FIX::SessionID &sessionId) noexcept(false) {
  FIX::MessageCracker::crack(msg, sessionId);
  spdlog::trace("fromApp, session Id [{}], message [{}]", sessionId.toString(),
                replaceSoh(msg.toString()));
}

void FixApp::onMessage(const FIX44::MarketDataSnapshotFullRefresh &m,
                       const FIX::SessionID &sessionID) {
  orderQueue_.enqueue(std::make_shared<const FIX44::MarketDataSnapshotFullRefresh>(m));
}
void FixApp::onMessage(const FIX44::MarketDataIncrementalRefresh &m,
                       const FIX::SessionID &sessionID) {
  orderQueue_.enqueue(std::make_shared<const FIX44::MarketDataIncrementalRefresh>(m));
}

// PUBLIC

FixApp::FixApp(const std::vector<std::string> &symbols, std::unique_ptr<IAuth> auth)
    : symbols_(symbols), auth_(std::move(auth)) {}

void FixApp::subscribeToDepth(const FIX::SessionID &sessionId) const {
  spdlog::debug("Subscribe to depth");
  FIX44::MarketDataRequest marketDataRequest;

  // Generate a unique request ID for this session's request
  const std::string reqId = "MDReq-" + std::to_string(std::time(nullptr));
  marketDataRequest.set(FIX::MDReqID(reqId));

  // Set subscription type (1 = Subscribe)
  marketDataRequest.set(
      FIX::SubscriptionRequestType(FIX::SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES));

  // Set market depth
  constexpr int BINANCE_MAX_DEPTH = 15;
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
  for (const auto &instrument : symbols_) {
    FIX44::MarketDataRequest::NoRelatedSym symbolGroup;
    symbolGroup.set(FIX::Symbol(instrument));
    marketDataRequest.addGroup(symbolGroup);
  }

  // Send the request to the corresponding market data session
  FIX::Session::sendToTarget(marketDataRequest, sessionId);
}

}  // namespace Binance
