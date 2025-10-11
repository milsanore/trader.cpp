#include "fix_app.h"

#include <quickfix/FixValues.h>
#include <quickfix/fix44/ExecutionReport.h>
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

#include "./../utils/threading.h"
#include "message_handling_mode.h"
#include "spdlog/spdlog.h"

namespace binance {

// PUBLIC

FixApp::FixApp(const std::vector<std::string>& symbols,
               std::unique_ptr<IAuth> auth,
               const int MAX_DEPTH)
    : symbols_(symbols), auth_(std::move(auth)), MAX_DEPTH_(MAX_DEPTH) {
  // TODO(mils): I think this is a singleton
  utils::Threading::set_thread_name(THREAD_NAME_);
  spdlog::info("naming FIXs own thread, name [{}], id [{}]", THREAD_NAME_,
               utils::Threading::get_os_thread_id());
}

void FixApp::subscribe_to_depth(const FIX::SessionID& session_id) const {
  spdlog::info("subscribing to depth. qualifier [{}], id [{}]",
               session_id.getSessionQualifier(), session_id.toString());

  FIX44::MarketDataRequest md_req;

  // Generate a unique request ID for this session's request
  const std::string req_id = "MDReq-" + std::to_string(std::time(nullptr));
  md_req.set(FIX::MDReqID(req_id));

  // Set subscription type (1 = Subscribe)
  md_req.set(
      FIX::SubscriptionRequestType(FIX::SubscriptionRequestType_SNAPSHOT_PLUS_UPDATES));

  // NB: Careful with Binance depth, use either 1 or 5000
  md_req.set(FIX::MarketDepth(MAX_DEPTH_));

  // Create NoMDEntryTypes group for requesting BID and OFFER
  FIX44::MarketDataRequest::NoMDEntryTypes e_types;
  // Add BID entry type (0)
  e_types.set(FIX::MDEntryType(FIX::MDEntryType_BID));
  md_req.addGroup(e_types);
  // Add OFFER entry type (1)
  e_types.set(FIX::MDEntryType(FIX::MDEntryType_OFFER));
  md_req.addGroup(e_types);
  // Add TRADE entry type (2)
  // e_types.set(FIX::MDEntryType(FIX::MDEntryType_TRADE));
  // md_req.addGroup(e_types);

  // Add symbols
  for (const auto& instrument : symbols_) {
    FIX44::MarketDataRequest::NoRelatedSym group;
    group.set(FIX::Symbol(instrument));
    md_req.addGroup(group);
  }

  // Send the request to the corresponding market data session
  FIX::Session::sendToTarget(md_req, session_id);
}

// PRIVATE

/// @brief Better FIX message logging: replaces SOH with '|'
static std::string replace_soh(const std::string& input) {
  std::string output = input;
  // Remove trailing SOH if present
  if (!output.empty() && output.back() == '\x01') {
    output.pop_back();
  }
  std::ranges::replace(output.begin(), output.end(), '\x01', '|');
  return output;
}

void FixApp::onCreate(const FIX::SessionID& sessionId) {
  spdlog::info("session created. qualifier [{}], id [{}]",
               sessionId.getSessionQualifier(), sessionId.toString());
};
void FixApp::onLogon(const FIX::SessionID& sessionId) {
  spdlog::info("Session logon, qualifier [{}], id [{}]", sessionId.getSessionQualifier(),
               sessionId.toString());
  // logon successful, nullify access keys
  // TODO: now need to wait for all sessions to be logged on before nullifying keys
  // auth_->clear_keys();

  if (sessionId.getSessionQualifier() == PRICE_SESSION_QUALIFIER_) {
    subscribe_to_depth(sessionId);
  } else if (sessionId.getSessionQualifier() == TRADE_SESSION_QUALIFIER_) {
    // subscribe to market trades
  } else if (sessionId.getSessionQualifier() == ORDER_SESSION_QUALIFIER_) {
    // do nothing for order session
  } else {
    spdlog::error("unknown session, qualifier [{}], id [{}]",
                  sessionId.getSessionQualifier(), sessionId.toString());
  }
};
void FixApp::onLogout(const FIX::SessionID& sessionId) {
  spdlog::info("session logout. qualifier [{}], id [{}]", sessionId.getSessionQualifier(),
               sessionId.toString());
};

void FixApp::toAdmin(FIX::Message& msg, const FIX::SessionID& sessionId) {
  const FIX::Header& header = msg.getHeader();
  FIX::MsgType msg_type;
  header.getField(msg_type);
  if (msg_type.getString() == static_cast<const char*>(FIX::MsgType_Logon)) {
    spdlog::info("authenticating. session qualifier [{}], session id [{}]",
                 sessionId.getSessionQualifier(), sessionId.toString());

    // collect required fields
    const std::string sender = header.getField(FIX::FIELD::SenderCompID);
    const std::string target = header.getField(FIX::FIELD::TargetCompID);
    const std::string seq_num = msg.getHeader().getField(FIX::FIELD::MsgSeqNum);
    const auto sending_time = FIX::UtcTimeStamp{};

    // construct payload for signing
    const std::string payload = std::string{"A"} + '\x01' + sender + '\x01' + target +
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
  } else {
    spdlog::info(
        "toAdmin.   session qualifier [{}], session id [{}], type [{}], message [{}]",
        sessionId.getSessionQualifier(), sessionId.toString(), msg_type.getString(),
        replace_soh(msg.toString()));
  }
};
void FixApp::toApp(FIX::Message& msg, const FIX::SessionID& sessionId) noexcept(false) {
  const FIX::Header& header = msg.getHeader();
  FIX::MsgType msg_type;
  header.getField(msg_type);
  spdlog::info("toApp. session qualifier [{}], session id [{}], type [{}], message [{}]",
               sessionId.getSessionQualifier(), sessionId.toString(),
               msg_type.getString(), replace_soh(msg.toString()));
};

void FixApp::fromAdmin(const FIX::Message& msg,
                       const FIX::SessionID& sessionId) noexcept(false) {
  const FIX::Header& header = msg.getHeader();
  FIX::MsgType msg_type;
  header.getField(msg_type);
  spdlog::info(
      "fromAdmin. session qualifier [{}], session id [{}], type [{}], message [{}]",
      sessionId.getSessionQualifier(), sessionId.toString(), msg_type.getString(),
      replace_soh(msg.toString()));
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
void FixApp::onMessage(const FIX44::ExecutionReport& message, const FIX::SessionID&) {
  spdlog::info("execution report");

  try {
    // Extract the ClOrdID
    FIX::ClOrdID clOrdID;
    message.get(clOrdID);
    std::string id = clOrdID.getValue();

    // Get execution type
    FIX::ExecType execType;
    message.get(execType);
    char execTypeValue = execType.getValue();

    // Extract the used and received quantities
    FIX::CumQty cumQty;  // Total number of base asset traded on this order.
    message.get(cumQty);

    FIX::QtyField cumQuoteQty(
        25017);  // Total number of quote asset traded on this order.
    message.getField(cumQuoteQty);
    std::string qty = cumQuoteQty.getString();

    FIX::Side side;
    message.get(side);

  } catch (const std::exception& e) {
    throw std::runtime_error(
        std::format("error processing execution report: [{}]", std::string(e.what())));
  }
}
// error catch-all
void onMessage(const FIX::Message& msg, const FIX::SessionID&) {
  spdlog::error("received unexpected message. payload [{}] ",
                replace_soh(msg.toString()));
}

}  // namespace binance
