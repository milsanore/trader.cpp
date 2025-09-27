#include "Auth.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <sodium.h>
#include <sodium/core.h>
#include <sodium/crypto_sign.h>
#include <sodium/utils.h>

#include <algorithm>
#include <cassert>
#include <format>
#include <fstream>
#include <gsl/gsl>
#include <iomanip>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

namespace Binance {

Auth::Auth(std::string &apiKey, std::string &privatePemPath)
    : apiKey_(apiKey), privatePemPath_(privatePemPath) {
  if (sodium_init() < 0) {
    throw std::runtime_error("libsodium failed to initialize");
  }
}

const std::string &Auth::getApiKey() const { return apiKey_; }

std::string Auth::signPayload(const std::string &payload) {
  const std::vector<unsigned char> seed = getSeedFromPem();
  const std::string signature = signPayload(payload, seed);
  return signature;
}

std::vector<unsigned char> Auth::getSeedFromPem() const {
  // fopen is unsafe, wrap in RAII
  const auto fileCloser = [](gsl::owner<FILE *> fp) {
    if (fp) fclose(fp);
  };
  std::unique_ptr<FILE, decltype(fileCloser)> fp(fopen(privatePemPath_.c_str(), "r"),
                                                 fileCloser);
  if (!fp) {
    throw std::runtime_error("Failed to open PEM file");
  }

  // PEM_read_PrivateKey is unsafe, wrap in RAII
  const auto keyCloser = [](EVP_PKEY *pkey) {
    if (pkey) EVP_PKEY_free(pkey);
  };
  std::unique_ptr<EVP_PKEY, decltype(keyCloser)> pkey(
      PEM_read_PrivateKey(fp.get(), nullptr, nullptr, nullptr), keyCloser);
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
std::string Auth::signPayload(const std::string &payload,
                              const std::vector<unsigned char> &seed) {
  std::array<unsigned char, crypto_sign_PUBLICKEYBYTES> pk{};
  std::array<unsigned char, crypto_sign_SECRETKEYBYTES> sk{};  // 64 bytes

  // Generate keypair from seed
  if (crypto_sign_seed_keypair(pk.data(), sk.data(), seed.data()) != 0) {
    throw std::runtime_error("Failed to generate keypair from seed");
  }

  std::array<unsigned char, crypto_sign_BYTES> sig{};
  if (crypto_sign_detached(
          sig.data(), nullptr,
          static_cast<const unsigned char *>(static_cast<const void *>(payload.data())),
          payload.size(), sk.data()) != 0) {
    throw std::runtime_error("Failed to sign payload");
  }

  // Base64 encode signature

  std::array<char, crypto_sign_BYTES * 2> b64{};
  sodium_bin2base64(b64.data(), sizeof(b64), sig.data(), sizeof(sig),
                    sodium_base64_VARIANT_ORIGINAL_NO_PADDING);

  return {b64.data()};
}

void Auth::clearKeys() {
  // logon successful, nullify access keys
  std::ranges::fill(apiKey_, 0);
  apiKey_.clear();
  std::ranges::fill(privatePemPath_, 0);
  privatePemPath_.clear();
}

}  // namespace Binance
