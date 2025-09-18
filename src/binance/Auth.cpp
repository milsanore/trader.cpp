#include <algorithm>
#include <cassert>
#include <fstream>
#include <format>
#include <iomanip>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>
#include <sodium.h>
#include <sodium/core.h>
#include <sodium/crypto_sign.h>
#include <sodium/utils.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include "Auth.h"

namespace Binance {

// static member function
std::vector<unsigned char> Auth::getSeedFromPem(const std::string& pemPath) {
	FILE* fp = fopen(pemPath.c_str(), "r");
	if (!fp) throw std::runtime_error("Failed to open PEM file");
	EVP_PKEY* pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
	fclose(fp);
	if (!pkey) throw std::runtime_error("Failed to read private key from PEM");

	size_t len = 32; // Ed25519 private key seed size
	std::vector<unsigned char> seed(len);
	if (EVP_PKEY_get_raw_private_key(pkey, seed.data(), &len) != 1)
		throw std::runtime_error("Failed to get raw private key");
	if (len != 32)
		throw std::runtime_error("Unexpected private key length");

	EVP_PKEY_free(pkey);
	return seed;
}

// static member function
std::string Auth::signPayload(const std::string& payload, const std::vector<unsigned char>& seed) {
	unsigned char pk[crypto_sign_PUBLICKEYBYTES];
	unsigned char sk[crypto_sign_SECRETKEYBYTES]; // 64 bytes

	// Generate keypair from seed
	if (crypto_sign_seed_keypair(pk, sk, seed.data()) != 0)
		throw std::runtime_error("Failed to generate keypair from seed");

	unsigned char sig[crypto_sign_BYTES];
	if (crypto_sign_detached(sig,
							nullptr,
							reinterpret_cast<const unsigned char*>(payload.data()),
							payload.size(),
							sk) != 0) {
		throw std::runtime_error("Failed to sign payload");
	}

	// Base64 encode signature
	char b64[crypto_sign_BYTES * 2];
	sodium_bin2base64(b64, sizeof(b64), sig, sizeof(sig), sodium_base64_VARIANT_ORIGINAL_NO_PADDING);

	return b64;
}

}
