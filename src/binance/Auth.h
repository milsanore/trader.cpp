#ifndef BINANCEAUTH_H
#define BINANCEAUTH_H

#include <vector>
#include <string>

namespace Binance {

/// @brief Binance's proprietary FIX authentication mechanism - helper functions
class Auth final {
public:
    /// Fetch a 32-byte Ed25519 seed from a private key PEM file (using OpenSSL)
    ///
    /// @param pemPath path to a private-key PEM file
    /// @return 32-byte Ed25519 seed
    static std::vector<unsigned char> getSeedFromPem(const std::string& pemPath);

    /// Generate a payload signature using a private key.
    /// Expand the key to a 64-byte secret key (using libsodium), sign a payload, output base64 signature.
    ///
    /// @param payload payload to be signed
    /// @param seed a 32-byte Ed25519 seed
    /// @return base64 payload signature
    static std::string signPayload(const std::string& payload, const std::vector<unsigned char>& seed);
};

}

#endif  // BINANCEAUTH_H
