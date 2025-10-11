#include "auth.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <sodium.h>
#include <sodium/core.h>
#include <sodium/crypto_sign.h>
#include <sodium/utils.h>

#include <algorithm>
#include <cassert>
#include <format>
#include <gsl/gsl>
#include <iomanip>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

namespace binance {

Auth::Auth(std::string& api_key, std::string& private_pem_path)
    : api_key_(api_key), private_pem_path_(private_pem_path) {
  if (sodium_init() < 0) {
    throw std::runtime_error("libsodium failed to initialize");
  }
}

const std::string& Auth::get_api_key() const {
  return api_key_;
}

std::string Auth::sign_payload(const std::string& payload) {
  const std::vector<unsigned char> seed = get_seed_from_pem();
  const std::string signature = sign_payload(payload, seed);
  return signature;
}

std::vector<unsigned char> Auth::get_seed_from_pem() const {
  std::lock_guard lock(mutex_);

  // fopen is unsafe, wrap in RAII
  const auto file_closer = [](gsl::owner<FILE*> fp) {
    if (fp) {
      fclose(fp);
    }
  };
  std::unique_ptr<FILE, decltype(file_closer)> fp(fopen(private_pem_path_.c_str(), "r"),
                                                  file_closer);
  if (!fp) {
    throw std::runtime_error("Failed to open PEM file");
  }

  // PEM_read_PrivateKey is unsafe, wrap in RAII
  const auto key_closer = [](EVP_PKEY* pkey) {
    if (pkey) {
      EVP_PKEY_free(pkey);
    }
  };
  std::unique_ptr<EVP_PKEY, decltype(key_closer)> pkey(
      PEM_read_PrivateKey(fp.get(), nullptr, nullptr, nullptr), key_closer);
  if (!pkey) {
    throw std::runtime_error("Failed to read private key from PEM");
  }

  constexpr size_t ED25519_SEED_SIZE = 32;
  size_t len = ED25519_SEED_SIZE;
  std::vector<unsigned char> seed(ED25519_SEED_SIZE);
  if (EVP_PKEY_get_raw_private_key(pkey.get(), seed.data(), &len) != 1) {
    throw std::runtime_error("Failed to get raw private key");
  }
  if (len != ED25519_SEED_SIZE) {
    throw std::runtime_error(
        std::format("Unexpected private key length, length [{}]", len));
  }

  return seed;
}

// static
std::string Auth::sign_payload(const std::string& payload,
                               const std::vector<unsigned char>& seed) {
  std::array<unsigned char, crypto_sign_PUBLICKEYBYTES> pk{};
  std::array<unsigned char, crypto_sign_SECRETKEYBYTES> sk{};  // 64 bytes

  // Generate keypair from seed
  if (crypto_sign_seed_keypair(pk.data(), sk.data(), seed.data()) != 0) {
    throw std::runtime_error("Failed to generate keypair from seed");
  }

  std::array<unsigned char, crypto_sign_BYTES> sig{};
  if (crypto_sign_detached(
          sig.data(), nullptr,
          static_cast<const unsigned char*>(static_cast<const void*>(payload.data())),
          payload.size(), sk.data()) != 0) {
    throw std::runtime_error("Failed to sign payload");
  }

  // Base64 encode signature

  std::array<char, crypto_sign_BYTES * 2> b64{};
  sodium_bin2base64(b64.data(), sizeof(b64), sig.data(), sizeof(sig),
                    sodium_base64_VARIANT_ORIGINAL_NO_PADDING);

  return {b64.data()};
}

void Auth::clear_keys() {
  // logon successful, nullify access keys
  std::ranges::fill(api_key_, 0);
  api_key_.clear();
  std::ranges::fill(private_pem_path_, 0);
  private_pem_path_.clear();
}

}  // namespace binance
