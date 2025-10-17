#include "binance/symbol.h"

#include <gtest/gtest.h>

using binance::Symbol;
using binance::SymbolEnum;

TEST(Symbol, FromStr_ValidInput_BTCUSDT) {
  SymbolEnum symbol = Symbol::from_str("BTCUSDT");
  EXPECT_EQ(symbol, SymbolEnum::BTCUSDT);
}

TEST(Symbol, FromStr_ValidInput_ETHUSDT) {
  SymbolEnum symbol = Symbol::from_str("ETHUSDT");
  EXPECT_EQ(symbol, SymbolEnum::ETHUSDT);
}

TEST(Symbol, FromStr_InvalidSize_TooShort) {
  EXPECT_THROW({ Symbol::from_str("BTC"); }, std::runtime_error);
}

TEST(Symbol, FromStr_InvalidSize_TooLong) {
  EXPECT_THROW({ Symbol::from_str("BTCUSDTX"); }, std::runtime_error);
}

TEST(Symbol, FromStr_UnknownSymbol) {
  EXPECT_THROW({ Symbol::from_str("XRPUSDT"); }, std::runtime_error);
}

TEST(Symbol, ToStrView_BTCUSDT) {
  std::string_view result = Symbol::to_str_view(SymbolEnum::BTCUSDT);
  EXPECT_EQ(result, "BTCUSDT");
}

TEST(Symbol, ToStr_BTCUSDT) {
  std::string result = Symbol::to_str(SymbolEnum::BTCUSDT);
  EXPECT_EQ(result, "BTCUSDT");
}

TEST(Symbol, ToStrView_ETHUSDT) {
  std::string_view result = Symbol::to_str_view(SymbolEnum::ETHUSDT);
  EXPECT_EQ(result, "ETHUSDT");
}

TEST(Symbol, ToStr_ETHUSDT) {
  std::string result = Symbol::to_str(SymbolEnum::ETHUSDT);
  EXPECT_EQ(result, "ETHUSDT");
}

TEST(Symbol, ToStr_InvalidEnum_Throws) {
  // Define an invalid enum value that's not in the switch.
  SymbolEnum invalid = static_cast<SymbolEnum>(999);
  EXPECT_THROW({ Symbol::to_str(invalid); }, std::runtime_error);
}

TEST(Symbol, ToUint_ValidEnumValues) {
  EXPECT_EQ(Symbol::to_uint(SymbolEnum::BTCUSDT), 0);
  EXPECT_EQ(Symbol::to_uint(SymbolEnum::ETHUSDT), 1);
}
