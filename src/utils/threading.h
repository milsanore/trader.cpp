#pragma once

#include <cstdint>
#include <string>

namespace utils {

class Threading {
 public:
  /// @brief Sets the current thread's name (visible in debuggers and some profilers).
  /// This is useful for debugging and monitoring multi-threaded applications.
  /// The maximum allowed name length depends on the platform:
  /// - Linux: 15 characters + null terminator
  /// - macOS: 63 characters + null terminator
  /// - Windows: UTF-16 string, shown in debuggers that support `SetThreadDescription`.
  /// @param name Name to assign to the current thread.
  static void set_thread_name(const std::string& name);

  /// @brief Returns the operating system thread ID for the current thread.
  /// This ID matches what is shown in system tools like `htop` (Linux),
  /// Activity Monitor (macOS), or Task Manager (Windows), and what `spdlog` uses
  /// in its default logging pattern (`%t`).
  /// @return A platform-specific thread identifier as an unsigned 64-bit integer.
  static uint64_t get_os_thread_id();
  static inline constexpr uint64_t ERROR_THREAD_ID = 9999999;
};

}  // namespace utils
