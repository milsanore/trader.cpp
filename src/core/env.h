#pragma once

#include <format>
#include <stdexcept>
#include <string>

namespace core {

struct Env {
 public:
  /// @brief load a variable from the environment, if it's not available, panic
  /// @param key the name of the environment variable
  /// @return env var string
  static std::string get_env_or_throw(const char* key) {
    if (const char* val = std::getenv(key)) {
      return {val};
    }
    throw std::runtime_error(std::format("envvar not defined, key [{}]", key));
  };
};

}  // namespace core
