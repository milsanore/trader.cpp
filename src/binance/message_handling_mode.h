#pragma once

#include <string>

namespace binance {

/// @brief Binance Message Handling Mode
/// Controls how the matching engine processes your messages
/// [More
/// info](https://developers.binance.com/docs/binance-spot-api-docs/fix-api#on-message-processing-order)
enum class MessageHandlingMode : uint16_t {
  /// @brief UNORDERED(1) - Messages from the client are allowed to be sent to
  /// the matching engine in any order
  UNORDERED = 1,
  /// @brief SEQUENTIAL(2) - Messages from the client are always sent to the
  /// matching engine in MsgSeqNum (34) order
  SEQUENTIAL = 2,
  /// @brief FIELD_ID(25035) - The Field ID that Binance
  /// uses for "Message Handling Mode"
  FIELD_ID = 25035,
};

uint16_t to_int(const MessageHandlingMode m) {
  return static_cast<uint16_t>(m);
}

std::string to_string(const MessageHandlingMode m) {
  return std::to_string(static_cast<uint16_t>(m));
}

}  // namespace binance
