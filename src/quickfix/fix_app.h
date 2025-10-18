#pragma once

#include <quickfix/Application.h>
#include <quickfix/SessionID.h>
#include <quickfix/fix44/Message.h>
#include <quickfix/fix44/MessageCracker.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../binance/iauth.h"
#include "concurrentqueue.h"

namespace quickfix {

/// @brief Binance FIX App - Manages FIX connectivity to Binance
class FixApp final : public FIX::Application, public FIX44::MessageCracker {
 public:
  FixApp(const std::vector<std::string>& symbols,
         std::unique_ptr<binance::IAuth> auth,
         const uint16_t MAX_DEPTH);
  ~FixApp() override = default;

  // Use the FIX44::MessageCracker to pull in the relevant overloads. This resolves the
  // ambiguity caused by multiple base classes having the same function name.
  using FIX44::MessageCracker::onMessage;

  /// @brief
  void subscribe_to_prices(const FIX::SessionID& session_id) const;
  /// @brief
  void subscribe_to_trades(const FIX::SessionID& session_id) const;

  /// @brief queue of market messages from Binance
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> order_queue_;
  /// @brief queue of trade messages from Binance
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> trade_queue_;

  // TODO: performance implication of a polymorphic queue
  // TODO: perhaps better to run two queues, or something else
  // TODO: we will have one of these per instrument

 private:
  static constexpr std::string THREAD_NAME_ = "tradercppFIX2";
  static constexpr std::string PRICE_SESSION_QUALIFIER_ = "PX";
  static constexpr std::string TRADE_SESSION_QUALIFIER_ = "TX";
  static constexpr std::string ORDER_SESSION_QUALIFIER_ = "OX";
  const std::vector<std::string>& symbols_;
  const std::unique_ptr<binance::IAuth> auth_;
  const uint16_t MAX_DEPTH_;

  void onCreate(const FIX::SessionID&) override;
  void onLogon(const FIX::SessionID&) override;
  void onLogout(const FIX::SessionID&) override;
  void toAdmin(FIX::Message&, const FIX::SessionID&) override;
  void toApp(FIX::Message&, const FIX::SessionID&) noexcept(false) override;
  void fromAdmin(const FIX::Message&, const FIX::SessionID&) noexcept(false) override;
  void fromApp(const FIX::Message&, const FIX::SessionID&) noexcept(false) override;

  // Callbacks for specific message types / MessageCracker overloads
  void onMessage(const FIX44::MarketDataSnapshotFullRefresh&,
                 const FIX::SessionID&) override;
  void onMessage(const FIX44::MarketDataIncrementalRefresh&,
                 const FIX::SessionID&) override;
  void onMessage(const FIX44::ExecutionReport&, const FIX::SessionID&) override;
};

}  // namespace quickfix
