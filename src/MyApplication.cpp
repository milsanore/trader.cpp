
#include <sodium.h>
#include <sodium/core.h>
#include <sodium/crypto_sign.h>
#include <sodium/utils.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <format>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <vector>
#include <stdexcept>

#include "MyApplication.h"

MyApplication::MyApplication(const std::string& apiKey, const std::string& privKeyPath)
	: apiKey_(apiKey), privKeyPath_(privKeyPath) {
	if (sodium_init() < 0) throw std::runtime_error("libsodium failed to initialize");
}

void MyApplication::onCreate(const FIX::SessionID& sessionId) {
	std::cout << std::format("SESSION CREATED [{}]", sessionId.toString()) << std::endl;
};
void MyApplication::onLogon(const FIX::SessionID& sessionId) {
	std::cout << std::format("SESSION LOGON [{}]", sessionId.toString()) << std::endl;
};
void MyApplication::onLogout(const FIX::SessionID& sessionId) {
	std::cout << std::format("SESSION LOGOUT [{}]", sessionId.toString()) << std::endl;
};
void MyApplication::toAdmin(FIX::Message& msg, const FIX::SessionID& sessionId) {
	std::cout << std::format("TO ADMIN, SESSION ID [{}], MESSAGE [{}]", sessionId.toString(), msg.toString()) << std::endl;
	auto& header = msg.getHeader();
	FIX::MsgType msgType;
	header.getField(msgType);
	if (msgType == FIX::MsgType_Logon) {
		// Collect required fields
		std::string sender = header.getField(FIX::FIELD::SenderCompID);
        std::string target = header.getField(FIX::FIELD::TargetCompID);
		std::string seqNum = msg.getHeader().getField(FIX::FIELD::MsgSeqNum);

		// std::string time = header.getField(FIX::FIELD::SendingTime);
		auto sendingTime = FIX::UtcTimeStamp(); // current timestamp
		msg.getHeader().setField(FIX::SendingTime(sendingTime)); // explicitly set

		// Construct payload for signing
		std::string payload = std::string("A")
							+ '\x01' + sender
							+ '\x01' + target
							+ '\x01' + seqNum
							+ '\x01' + FIX::UtcTimeStampConvertor::convert(sendingTime);

		std::vector<unsigned char> seed = loadPrivateKey(privKeyPath_);
		std::string signature = signPayload(payload, seed);
		
		msg.setField(FIX::Username(apiKey_));
		msg.setField(FIX::RawData(signature));
		msg.setField(FIX::RawDataLength(signature.length()));
		// Binance Message Handling Mode,
		// controls how the matching engine processes your messages
		// UNORDERED(1)  - Messages from the client are allowed to be sent to the matching engine in any order
		// SEQUENTIAL(2) - Messages from the client are always sent to the matching engine in MsgSeqNum (34) order
		// TODO: Enum
        msg.setField(FIX::StringField(25035, "2"));
	}
};
void MyApplication::toApp(FIX::Message& msg, const FIX::SessionID& sessionId) noexcept(false) {
	std::cout << std::format("TO APP, SESSION ID [{}], MESSAGE [{}]", sessionId.toString(), msg.toString()) << std::endl;
};
void MyApplication::fromAdmin(const FIX::Message& msg, const FIX::SessionID& sessionId) noexcept(false) {
	std::cout << std::format("FROM ADMIN, SESSION ID [{}], MESSAGE [{}]", sessionId.toString(), msg.toString()) << std::endl;
};
void MyApplication::fromApp(const FIX::Message& msg, const FIX::SessionID& sessionId) noexcept(false) {
	FIX::MessageCracker::crack(msg, sessionId);
	std::cout << std::format("FROM APP, SESSION ID [{}], MESSAGE [{}]", sessionId.toString(), msg.toString()) << std::endl;
};


std::vector<unsigned char> MyApplication::loadPrivateKey(const std::string& pemPath) {
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

std::string MyApplication::signPayload(const std::string& payload, const std::vector<unsigned char>& key) {
    unsigned char pk[crypto_sign_PUBLICKEYBYTES];
    unsigned char sk[crypto_sign_SECRETKEYBYTES]; // 64 bytes

    // Generate keypair from seed
    if (crypto_sign_seed_keypair(pk, sk, key.data()) != 0)
        throw std::runtime_error("Failed to generate keypair from seed");

    unsigned char sig[crypto_sign_BYTES];

    if (crypto_sign_detached(sig,
							nullptr,
							reinterpret_cast<const unsigned char*>(payload.data()),
							payload.size(),
							sk) != 0)
        throw std::runtime_error("Failed to sign payload");

    // Base64 encode signature
    char b64[crypto_sign_BYTES * 2];
    sodium_bin2base64(b64, sizeof(b64), sig, sizeof(sig), sodium_base64_VARIANT_ORIGINAL_NO_PADDING);

    return std::string(b64);
}
