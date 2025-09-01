#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <quickfix/Application.h>
#include <quickfix/SessionID.h>

class MyApplication : public FIX::Application {
public:
	MyApplication() = default;
	~MyApplication() override = default;

	void onCreate(const FIX::SessionID&) override;
	void onLogon(const FIX::SessionID&) override;
	void onLogout(const FIX::SessionID&) override;
	void toAdmin(FIX::Message&, const FIX::SessionID&) override;
	void toApp(FIX::Message&, const FIX::SessionID&) noexcept(false) override;
	void fromAdmin(const FIX::Message&, const FIX::SessionID&) noexcept(false) override;
	void fromApp(const FIX::Message&, const FIX::SessionID&) noexcept(false) override;
};

#endif  // MYAPPLICATION_H
