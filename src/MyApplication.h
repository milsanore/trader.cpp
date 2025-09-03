#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <quickfix/Application.h>
#include <quickfix/SessionID.h>
#include <quickfix/MessageCracker.h>
#include <openssl/evp.h>

class MyApplication : public FIX::Application, public FIX::MessageCracker {
public:
	MyApplication(const std::string& apiKey, const std::string& privKeyPath);
	~MyApplication() override = default;
	void onCreate(const FIX::SessionID&) override;
	void onLogon(const FIX::SessionID&) override;
	void onLogout(const FIX::SessionID&) override;
	void toAdmin(FIX::Message&, const FIX::SessionID&) override;
	void toApp(FIX::Message&, const FIX::SessionID&) noexcept(false) override;
	void fromAdmin(const FIX::Message&, const FIX::SessionID&) noexcept(false) override;
	void fromApp(const FIX::Message&, const FIX::SessionID&) noexcept(false) override;

private:
	// Load private key
	// Load an Ed25519 private key from a PEM file (using OpenSSL) and extract raw 32-byte seed
	std::vector<unsigned char> loadPrivateKey(const std::string& pemPath);
	// Sign a message using private key
	// Expand key to a 64-byte secret key (using libsodium), sign a payload, output base64 signature
	std::string signPayload(const std::string& payload, const std::vector<unsigned char>& key);

	std::string apiKey_, privKeyPath_;
};

#endif  // MYAPPLICATION_H
