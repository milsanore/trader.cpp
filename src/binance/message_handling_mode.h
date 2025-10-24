#pragma once

#include <cstdint>
#include <string>

namespace binance {

/// @brief Binance Message Handling Mode
/// Controls how the matching engine processes your messages
/// [More
/// info](https://developers.binance.com/docs/binance-spot-api-docs/fix-api#on-message-processing-order)
struct MessageHandlingMode {
  enum class Mode : uint16_t {
    /// @brief UNORDERED(1) - Messages from the client are allowed to be sent to
    /// the matching engine in any order
    UNORDERED = 1,
    /// @brief SEQUENTIAL(2) - Messages from the client are always sent to the
    /// matching engine in MsgSeqNum (34) order
    SEQUENTIAL = 2,
  };

  static inline constexpr uint16_t FIELD_ID = 25'035;

  static constexpr uint16_t to_int(Mode m) noexcept { return static_cast<uint16_t>(m); }

  static std::string to_string(Mode m) { return std::to_string(to_int(m)); }
};

}  // namespace binance
