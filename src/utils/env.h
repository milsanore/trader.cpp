#pragma once

#include <format>
#include <stdexcept>
#include <string>

namespace utils {

struct Env {
 public:
  /// @brief load a variable from the environment, if it's not available, panic
  /// @param key the name of the environment variable
  /// @return env var string
  static std::string get_env_or_throw(const char* key) {
    if (const char* val = std::getenv(key)) {
      if (const auto valStr = std::string{val}; valStr.empty()) {
        throw std::runtime_error(std::format("empty envvar, key [{}]", key));
      }
      return {val};
    }
    throw std::runtime_error(std::format("envvar not defined, key [{}]", key));
  };
};

}  // namespace utils
