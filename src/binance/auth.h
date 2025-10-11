#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "iauth.h"

namespace binance {

/// @brief Binance's proprietary FIX authentication mechanism - helper functions
class Auth final : public IAuth {
 public:
  Auth(std::string& api_key, std::string& private_pem_path);

  /// Generate a payload signature using a private key.
  /// Expand the key to a 64-byte secret key (using libsodium), sign a payload,
  /// output base64 signature.
  /// This function will fetch the PEM file from disk.
  /// @param payload payload to be signed
  /// @return base64 payload signature
  std::string sign_payload(const std::string& payload) override;

  /// Generate a payload signature using a private key.
  /// Expand the key to a 64-byte secret key (using libsodium), sign a payload,
  /// output base64 signature.
  /// @param payload payload to be signed
  /// @param seed a 32-byte Ed25519 seed
  /// @return base64 payload signature
  static std::string sign_payload(const std::string& payload,
                                  const std::vector<unsigned char>& seed);

  const std::string& get_api_key() const override;

  void clear_keys() override;

 private:
  // when authenticating with Binance, each session authenticates independently,
  // and in parallel. requires synchronisation because of file access.
  mutable std::mutex mutex_;

  std::string& api_key_;
  std::string& private_pem_path_;
  /// Fetch a 32-byte Ed25519 seed from a private key PEM file (using OpenSSL)
  /// @return 32-byte Ed25519 seed
  std::vector<unsigned char> get_seed_from_pem() const;
};

}  // namespace binance
