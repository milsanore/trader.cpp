#pragma once

#include <charconv>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

namespace utils {

/// @brief Double-precision manipulation functions
struct Double {
 public:
  /// @brief Convert a double price to uint64 by multiplication and truncation
  /// @param value The double to convert to an int.
  /// NB: the max integer represented by a double is 2^53 (i.e. less than 2^64).
  /// @param ticks_per_unit the multiplier to apply. typically 1/tick_size.
  static inline uint64_t toUint64(const double value, const double ticks_per_unit) {
    const double scaled = value * ticks_per_unit;
    return static_cast<uint64_t>(scaled);
  }

  /// @brief pretty-print a double, efficiently
  /// - thousands separators (commas)
  /// - no trailing zeros
  /// - handles negative values correctly
  /// - efficient, allocation-friendly using std::to_chars and manual comma insertion
  /// - uses only stack memory and no extra allocations beyond the returned std::string
  /// @param val
  /// @return
  static std::string pretty(double val) {
    char buffer[64]{};
    // Convert double to string with max 15 significant digits
    auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), val,
                                   std::chars_format::general, 15);
    if (ec != std::errc()) {
      return {};  // conversion error fallback
    }
    std::string s(buffer, ptr);

    // Trim trailing zeros after decimal point
    if (auto dot_pos = s.find('.'); dot_pos != std::string::npos) {
      auto end = s.size() - 1;
      while (end > dot_pos && s[end] == '0') {
        --end;
      }
      if (end == dot_pos) {
        --end;  // remove decimal point if no decimals left
      }
      s.erase(end + 1);
    }

    // Insert commas for thousands in the integer part
    auto dot_pos = s.find('.');
    size_t int_end = (dot_pos == std::string::npos) ? s.size() : dot_pos;

    // Skip leading sign if negative
    size_t start = 0;
    if (!s.empty() && s[0] == '-') {
      start = 1;
    }

    // Insert commas from right to left
    for (int i = static_cast<int>(int_end) - 3; i > static_cast<int>(start); i -= 3) {
      s.insert(i, ",");
    }

    return s;
  }

  static std::string trim(double val) {
    // 64 bytes is usually more than enough for a double
    char buffer[64];
    auto [ptr, ec] =
        std::to_chars(buffer, buffer + sizeof(buffer), val, std::chars_format::fixed, 15);
    if (ec != std::errc()) {
      return {};  // conversion error
    }

    std::string s(buffer, ptr);

    // Remove trailing zeros and decimal point if needed
    auto dot_pos = s.find('.');
    if (dot_pos != std::string::npos) {
      size_t end = s.size() - 1;
      while (end > dot_pos && s[end] == '0') {
        --end;
      }
      if (end == dot_pos) {
        --end;
      }
      s.erase(end + 1);
    }

    return s;
  }
};

}  // namespace utils
