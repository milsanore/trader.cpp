#include "utils/double.h"

#include <gtest/gtest.h>

#include <string>

#include "binance/config.h"

using utils::Double;

// ::toUint64() -----------------------------------------------

TEST(Double_ToUint64, ZeroValue) {
  EXPECT_EQ(Double::toUint64(0.0, 100), 0u);
}

TEST(Double_ToUint64, OneValue_NoDecimals) {
  EXPECT_EQ(Double::toUint64(1.0, 1), 1u);
}

TEST(Double_ToUint64, OneValue_TwoDecimals) {
  EXPECT_EQ(Double::toUint64(1.0, 100), 100u);
}

TEST(Double_ToUint64, Fraction_TruncatesCorrectly) {
  // 1.234 * 100 = 123.4 -> truncates to 123
  EXPECT_EQ(Double::toUint64(1.234, 100), 123u);
}

TEST(Double_ToUint64, HighPrecision_TruncatesCorrectly) {
  // 0.12345678 * 1e8 = 12345678.0
  EXPECT_EQ(Double::toUint64(0.123'456'789, 100000000), 12'345'678u);
}

TEST(Double_ToUint64, SmallValue_LargeDecimals) {
  // 0.00000001 * 1e8 = 1
  EXPECT_EQ(Double::toUint64(0.000'000'01, 100000000), 1u);
}

TEST(Double_ToUint64, MaxValue_ZeroDecimals) {
  // the maximal mantissa of a double is 2^53. this is the biggest integer represented.
  double max_exact_double = 0x1p53;
  // 2^53 as a uint
  uint64_t max_exact_double_uint = 9'007'199'254'740'992ull;
  EXPECT_EQ(Double::toUint64(max_exact_double, 1), max_exact_double_uint);
}

TEST(Double_ToUint64, LargeValue_WithDecimals) {
  // 9007199254740.992 * 100 = 900719925474099.2 -> truncates to 900719925474099
  EXPECT_EQ(Double::toUint64(9'007'199'254'740.992, 100), 900'719'925'474'099ull);
}

TEST(Double_ToUint64, NegativeValue_ImplicitCast) {
  // Cast to uint64_t will wrap around since value is negative
  // It's undefined behavior by design here, but we can still test it.
  uint64_t max_val = UINT64_MAX;
  EXPECT_EQ(Double::toUint64(-1.0, 1), max_val);
}

// ::pretty() -----------------------------------------------

TEST(Double_Pretty, PositiveNumbers) {
  EXPECT_EQ(Double::pretty(1234567.890000), "1,234,567.89");
  EXPECT_EQ(Double::pretty(1000.0), "1,000");
  EXPECT_EQ(Double::pretty(123.456), "123.456");
  EXPECT_EQ(Double::pretty(0.00123), "0.00123");
  EXPECT_EQ(Double::pretty(0.0), "0");
}

TEST(Double_Pretty, NegativeNumbers) {
  EXPECT_EQ(Double::pretty(-1234567.890000), "-1,234,567.89");
  EXPECT_EQ(Double::pretty(-1000.0), "-1,000");
  EXPECT_EQ(Double::pretty(-0.0001), "-0.0001");
  EXPECT_EQ(Double::pretty(-0.0), "-0");  // negative zero prints as zero
}

TEST(Double_Pretty, TrailingZeros) {
  EXPECT_EQ(Double::pretty(1234.5000), "1,234.5");
  EXPECT_EQ(Double::pretty(1234.0000), "1,234");
  EXPECT_EQ(Double::pretty(1234.0001), "1,234.0001");
}

TEST(Double_Pretty, LargeNumbers) {
  EXPECT_EQ(Double::pretty(1234567890123.0), "1,234,567,890,123");
  EXPECT_EQ(Double::pretty(1234567890123.4567),
            "1,234,567,890,123.46");  // rounded by to_chars precision
}

TEST(Double_Pretty, SmallNumbers) {
  EXPECT_EQ(Double::pretty(1e-10), "1e,-10");  // scientific notation
  EXPECT_EQ(Double::pretty(-1e-10), "-1e,-10");
}

TEST(Double_Pretty, Zero) {
  EXPECT_EQ(Double::pretty(0.0), "0");
}

// ::trailing_zeroes() -----------------------------------------------

TEST(Double_Trim, WholeNumbers) {
  EXPECT_EQ(Double::trim(0.0), "0");
  EXPECT_EQ(Double::trim(123.0), "123");
  EXPECT_EQ(Double::trim(1000000.0), "1000000");
}

TEST(Double_Trim, TrailingZeros) {
  EXPECT_EQ(Double::trim(123.45000), "123.450000000000003");
  EXPECT_EQ(Double::trim(1.230000000000000), "1.23");
  EXPECT_EQ(Double::trim(123.000), "123");
  EXPECT_EQ(Double::trim(1000.100000), "1000.100000000000023");
}

TEST(Double_Trim, HighPrecisionDecimal) {
  EXPECT_EQ(Double::trim(1.234567890123456), "1.234567890123456");
  EXPECT_EQ(Double::trim(0.123456789012345), "0.123456789012345");
}

TEST(Double_Trim, NegativeNumbers) {
  EXPECT_EQ(Double::trim(-0.0), "-0");  // normalized as "0"
  EXPECT_EQ(Double::trim(-123.45000), "-123.450000000000003");
  EXPECT_EQ(Double::trim(-1000.0), "-1000");
  EXPECT_EQ(Double::trim(-0.0001), "-0.0001");
}

TEST(Double_Trim, SmallNumbersNoExponent) {
  EXPECT_EQ(Double::trim(0.00005), "0.00005");
  EXPECT_EQ(Double::trim(0.000000000123), "0.000000000123");
}

TEST(Double_Trim, EdgeCases) {
  EXPECT_EQ(Double::trim(1.0 / 3), "0.333333333333333");  // 15 digits
  EXPECT_EQ(Double::trim(-1.0 / 3), "-0.333333333333333");
}
