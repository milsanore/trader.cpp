#pragma once

#include <string>

namespace binance {

class IAuth {
 public:
  virtual ~IAuth() = default;
  /// @param payload payload to be signed
  /// @return base64 payload signature
  virtual std::string sign_payload(const std::string& payload) = 0;
  // returns the api key, used as a username with Binance
  virtual const std::string& get_api_key() const = 0;
  // once keys are no longer needed, they can be cleared from memory
  virtual void clear_keys() = 0;
};

}  // namespace binance
