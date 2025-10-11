#pragma once

#include <string>

namespace binance {

class IAuth {
 public:
  virtual ~IAuth() = default;

  /// @brief Generate a payload signature using a private key.
  /// @param payload payload to be signed
  /// @return base64 payload signature
  virtual std::string sign_payload(const std::string& payload) = 0;

  // returns the api key - used as a username by Binance
  virtual const std::string& get_api_key() const = 0;

  // clear keys from memory (after no longer needed)
  virtual void clear_keys() = 0;
};

}  // namespace binance
