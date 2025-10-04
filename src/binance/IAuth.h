#pragma once

#include <string>

namespace Binance {

class IAuth {
 public:
  virtual ~IAuth() = default;
  /// @param payload payload to be signed
  /// @return base64 payload signature
  virtual std::string signPayload(const std::string &payload) = 0;
  // returns the api key, used as a username with Binance
  virtual const std::string &getApiKey() const = 0;
  // once keys are no longer needed, they can be cleared from memory
  virtual void clearKeys() = 0;
};

}  // namespace Binance
