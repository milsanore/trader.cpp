#include <iostream>
#include <format>
#include "MyApplication.h"

void MyApplication::onCreate(const FIX::SessionID&) {
	std::cout << std::format("CREATED!") <<  std::endl;
};
void MyApplication::onLogon(const FIX::SessionID&) {
	std::cout << std::format("LOGON!") <<  std::endl;
};
void MyApplication::onLogout(const FIX::SessionID&) {
	std::cout << std::format("LOGOUT!") <<  std::endl;
};
void MyApplication::toAdmin(FIX::Message&, const FIX::SessionID&) {
	std::cout << std::format("TO ADMIN!") <<  std::endl;
};
void MyApplication::toApp(FIX::Message&, const FIX::SessionID&) noexcept(false) {
	std::cout << std::format("TO APP!") <<  std::endl;
};
void MyApplication::fromAdmin(const FIX::Message&, const FIX::SessionID&) noexcept(false) {
	std::cout << std::format("FROM ADMIN!") <<  std::endl;
};
void MyApplication::fromApp(const FIX::Message&, const FIX::SessionID&) noexcept(false) {
	std::cout << std::format("FROM APP!") <<  std::endl;
};
