#include "fix_app.h"

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

#include "message_handling_mode.h"
#include "spdlog/spdlog.h"

namespace binance {

// PRIVATE

// better FIX message logging
std::string replace_soh(const std::string& input) {
  std::string output = input;
  std::ranges::replace(output.begin(), output.end(), '\x01', '|');
  return output;
}

void FixApp::onCreate(const FIX::SessionID& sessionId) {
  spdlog::info("Session created, id [{}]", sessionId.toString());
};
void FixApp::onLogon(const FIX::SessionID& sessionId) {
  spdlog::info("Session logon, id [{}]", sessionId.toString());
  // logon successful, nullify access keys
  auth_->clear_keys();
  subscribe_to_depth(sessionId);
};
void FixApp::onLogout(const FIX::SessionID& sessionId) {
  spdlog::info("Session logout, id [{}]", sessionId.toString());
};
void FixApp::toAdmin(FIX::Message& msg, const FIX::SessionID& sessionId) {
  const FIX::Header& header = msg.getHeader();
  FIX::MsgType msg_type;
  header.getField(msg_type);
  spdlog::info("toAdmin, session Id [{}], type [{}], message [{}]", sessionId.toString(),
               msg_type.getString(), replace_soh(msg.toString()));

  if (msg_type.getString() == static_cast<const char*>(FIX::MsgType_Logon)) {
    spdlog::info("authenticating");

    // collect required fields
    const std::string sender = header.getField(FIX::FIELD::SenderCompID);
    const std::string target = header.getField(FIX::FIELD::TargetCompID);
    const std::string seq_num = msg.getHeader().getField(FIX::FIELD::MsgSeqNum);
    const auto sending_time = FIX::UtcTimeStamp();

    // construct payload for signing
    const std::string payload = std::string("A") + '\x01' + sender + '\x01' + target +
                                '\x01' + seq_num + '\x01' +
                                FIX::UtcTimeStampConvertor::convert(sending_time);
    const std::string signature = auth_->sign_payload(payload);
    assert(signature.size() <= INT_MAX);

    // assemble message
    msg.getHeader().setField(FIX::SendingTime(sending_time));
    msg.setField(FIX::Username(auth_->get_api_key()));
    msg.setField(FIX::RawData(signature));
    msg.setField(FIX::RawDataLength(static_cast<FIX::LENGTH>(signature.size())));
    msg.setField(FIX::StringField(to_int(MessageHandlingMode::FIELD_ID),
                                  to_string(MessageHandlingMode::SEQUENTIAL)));
  }
};
void FixApp::toApp(FIX::Message& msg, const FIX::SessionID& sessionId) noexcept(false) {
  const FIX::Header& header = msg.getHeader();
  FIX::MsgType msg_type;
  header.getField(msg_type);
  spdlog::debug("toApp, session Id [{}], type [{}], message [{}]", sessionId.toString(),
                msg_type.getString(), replace_soh(msg.toString()));
};
void FixApp::fromAdmin(const FIX::Message& msg,
                       const FIX::SessionID& sessionId) noexcept(false) {
  const FIX::Header& header = msg.getHeader();
  FIX::MsgType msg_type;
  header.getField(msg_type);
  spdlog::debug("fromAdmin, session Id [{}], type [{}], message [{}]",
                sessionId.toString(), msg_type.getString(), replace_soh(msg.toString()));
};
void FixApp::fromApp(const FIX::Message& msg,
                     const FIX::SessionID& sessionId) noexcept(false) {
  FIX::MessageCracker::crack(msg, sessionId);
}

void FixApp::onMessage(const FIX44::MarketDataSnapshotFullRefresh& m,
                       const FIX::SessionID& sessionID) {
  order_queue_.enqueue(std::make_shared<const FIX44::MarketDataSnapshotFullRefresh>(m));
}
void FixApp::onMessage(const FIX44::MarketDataIncrementalRefresh& m,
                       const FIX::SessionID& sessionID) {
  order_queue_.enqueue(std::make_shared<const FIX44::MarketDataIncrementalRefresh>(m));
}

// PUBLIC

FixApp::FixApp(const std::vector<std::string>& symbols, std::unique_ptr<IAuth> auth)
    : symbols_(symbols), auth_(std::move(auth)) {}

void FixApp::subscribe_to_depth(const FIX::SessionID& session_id) const {
  spdlog::debug("Subscribe to depth");
  FIX44::MarketDataRequest md_req;

  // Generate a unique request ID for this session's request
  const std::string req_id = "MDReq-" + std::to_string(std::time(nullptr));
  md_req.set(FIX::MDReqID(req_id));

  // Set subscription type (1 = Subscribe)
  md_req.set(
      FIX::SubscriptionRequestType(FIX::SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES));

  // Set market depth
  constexpr int BINANCE_MAX_DEPTH = 100;
  md_req.set(FIX::MarketDepth(BINANCE_MAX_DEPTH));

  // Create NoMDEntryTypes group for requesting BID and OFFER
  FIX44::MarketDataRequest::NoMDEntryTypes e_types;
  // Add BID entry type (0)
  e_types.set(FIX::MDEntryType(FIX::MDEntryType_BID));
  md_req.addGroup(e_types);
  // Add OFFER entry type (1)
  e_types.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
  md_req.addGroup(e_types);

  // Add symbols
  for (const auto& instrument : symbols_) {
    FIX44::MarketDataRequest::NoRelatedSym group;
    group.set(FIX::Symbol(instrument));
    md_req.addGroup(group);
  }

  // Send the request to the corresponding market data session
  FIX::Session::sendToTarget(md_req, session_id);
}

}  // namespace binance
