#include "binance/side.h"

#include <gtest/gtest.h>

using binance::Side;
using binance::SideEnum;

TEST(Side, FromStr_ValidInputs) {
  EXPECT_EQ(Side::from_str('1'), SideEnum::BUY);
  EXPECT_EQ(Side::from_str('2'), SideEnum::SELL);
}

TEST(Side, FromStr_InvalidInput_Throws) {
  EXPECT_THROW(Side::from_str('3'), std::runtime_error);
  EXPECT_THROW(Side::from_str('B'), std::runtime_error);
  EXPECT_THROW(Side::from_str('\0'), std::runtime_error);
}

TEST(Side, ToStr_ValidInputs) {
  EXPECT_EQ(Side::to_str_view(SideEnum::BUY), "BUY");
  EXPECT_EQ(Side::to_str_view(SideEnum::SELL), "SELL");
}

TEST(Side, ToChar_ValidInputs) {
  EXPECT_EQ(Side::to_char(SideEnum::BUY), '1');
  EXPECT_EQ(Side::to_char(SideEnum::SELL), '2');
}

TEST(Side, ToStr_InvalidEnum_Throws) {
  // Create an invalid enum value (not BUY or SELL)
  auto invalid = static_cast<SideEnum>(-1);
  EXPECT_THROW(Side::to_str_view(invalid), std::runtime_error);
}

TEST(Side, Roundtrip_CharToEnumToChar) {
  EXPECT_EQ(Side::to_char(Side::from_str('1')), '1');
  EXPECT_EQ(Side::to_char(Side::from_str('2')), '2');
}

TEST(Side, Roundtrip_CharToEnumToStr) {
  EXPECT_EQ(Side::to_str_view(Side::from_str('1')), "BUY");
  EXPECT_EQ(Side::to_str_view(Side::from_str('2')), "SELL");
}
