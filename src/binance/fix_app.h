#pragma once

#include <quickfix/Application.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/SessionID.h>
#include <quickfix/fix44/Message.h>

#include <memory>
#include <string>
#include <vector>

#include "concurrentqueue.h"
#include "iauth.h"

namespace binance {

/// @brief Binance FIX App - Manages FIX connectivity to Binance
class FixApp final : public FIX::Application, public FIX::MessageCracker {
 public:
  FixApp(const std::vector<std::string>& symbols,
         std::unique_ptr<IAuth> auth,
         int max_depth);
  ~FixApp() override = default;
  /// @brief
  void subscribe_to_depth(const FIX::SessionID& session_id) const;
  /// @brief queue of market messages from Binance
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> order_queue_;
  /// @brief queue of trade messages from Binance
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> trade_queue_;

  // TODO: performance implication of a polymorphic queue
  // TODO: perhaps better to run two queues, or something else
  // TODO: we will have one of these per instrument

 private:
  const std::vector<std::string>& symbols_;
  const std::unique_ptr<IAuth> auth_;
  const int MAX_DEPTH_;

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
};

}  // namespace binance
