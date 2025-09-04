#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <quickfix/Application.h>
#include <quickfix/SessionID.h>
#include <quickfix/MessageCracker.h>

class MyApplication final : public FIX::Application, public FIX::MessageCracker {
public:
	MyApplication(std::string apiKey, std::string privatePemPath);
	~MyApplication() override = default;
	void onCreate(const FIX::SessionID&) override;
	void onLogon(const FIX::SessionID&) override;
	void onLogout(const FIX::SessionID&) override;
	void toAdmin(FIX::Message&, const FIX::SessionID&) override;
	void toApp(FIX::Message&, const FIX::SessionID&) noexcept(false) override;
	void fromAdmin(const FIX::Message&, const FIX::SessionID&) noexcept(false) override;
	void fromApp(const FIX::Message&, const FIX::SessionID&) noexcept(false) override;

private:
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

	std::string apiKey_, privatePemPath_;
};

#endif  // MYAPPLICATION_H
