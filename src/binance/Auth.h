#ifndef BINANCEAUTH_H
#define BINANCEAUTH_H

#include <string>
#include <vector>
#include "IAuth.h"

namespace Binance {

/// @brief Binance's proprietary FIX authentication mechanism - helper functions
class Auth final : public IAuth {
public:
    Auth(std::string& apiKey, std::string& privatePemPath);
    std::string signPayload(const std::string& payload) override;
    /// Generate a payload signature using a private key.
    /// Expand the key to a 64-byte secret key (using libsodium), sign a payload, output base64 signature.
    /// @param payload payload to be signed
    /// @param seed a 32-byte Ed25519 seed
    /// @return base64 payload signature
    static std::string signPayload(const std::string& payload, const std::vector<unsigned char>& seed);
    const std::string& getApiKey() const override;
    void clearKeys() override;

private:
    std::string& apiKey_;
    std::string& privatePemPath_;
    /// Fetch a 32-byte Ed25519 seed from a private key PEM file (using OpenSSL)
    /// @return 32-byte Ed25519 seed
    std::vector<unsigned char> getSeedFromPem() const;
};

}

#endif  // BINANCEAUTH_H
