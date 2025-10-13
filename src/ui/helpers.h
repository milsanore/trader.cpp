#pragma once

#include <cmath>
#include <string>

namespace ui {

class Helpers {
 public:
  // Helper to pad or truncate a string to a fixed width
  static std::string Pad(const std::string& input, const size_t width) {
    if (input.size() >= width) {
      return input.substr(0, width);
    }

    std::string result = input;
    result.resize(width, ' ');
    return result;
  }

  // Helper to pad or truncate a string to a fixed width
  static std::string Pad(const double input, const size_t width) {
    return Pad(std::isnan(input) ? "" : std::to_string(input), width);
  }

  // Helper to pad or truncate a string to a fixed width
  static std::string Pad(const uint64_t input, const size_t width) {
    return Pad(std::to_string(input), width);
  }
};

}  // namespace ui
