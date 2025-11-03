#pragma once

#include <format>
#include <stdexcept>
#include <string>

#include "spdlog/spdlog.h"

namespace utils {

/// @brief OS environment helpers
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

  /// @brief log the architecutre in use
  static void log_current_architecture() {
    // detect architecture
#if defined(__aarch64__) || defined(_M_ARM64)
    constexpr const char* ARCH = "ARM64";
#elif defined(__arm__) || defined(_M_ARM)
    constexpr const char* ARCH = "ARM (32-bit)";
#elif defined(__x86_64__) || defined(_M_X64)
    constexpr const char* ARCH = "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    constexpr const char* ARCH = "x86 (32-bit)";
#else
    constexpr const char* ARCH = "unknown architecture";
#endif

    // detect OS
#if defined(_WIN32)
    constexpr const char* OS = "windows";
#elif defined(__APPLE__)
    constexpr const char* OS = "macOS";
#elif defined(__linux__)
    constexpr const char* OS = "linux";
#else
    constexpr const char* OS = "unknown OS";
#endif

    // log combined info
    spdlog::info("Compiled for [{}] on [{}]", ARCH, OS);
  }

  /// @brief Cache line size for the current architecture
  static inline constexpr std::size_t CACHE_LINE_SIZE =
#if defined(__aarch64__) || defined(_M_ARM64)
      128;
#elif defined(__x86_64__) || defined(_M_X64)
      64;
#else
      64;  // default/fallback cache line size
#endif
};

}  // namespace utils
