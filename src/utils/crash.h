#pragma once

#include <csignal>
#include <exception>
#include <new>

#include "spdlog/spdlog.h"

#ifdef _WIN32
#include <io.h>
#define write _write
#else
#include <unistd.h>
#endif

namespace utils {

/// @brief Portable crash handling utility
class Crash {
 public:
  /// @brief std::terminate handler
  static void handle_terminate() {
    spdlog::error("[CRITICAL] Unhandled exception or terminate called!");
    std::abort();  // ensure process exits
  }

  /// @brief std::new_handler for OOM
  static void handle_bad_alloc() {
    spdlog::error("[CRITICAL] Memory allocation failed (std::bad_alloc)");
    std::terminate();  // invokes terminate handler
  }

  /// @brief Signal handler (async-signal-safe)
  static void handle_signal(int sig) {
    constexpr char prefix[] = "CRITICAL: Fatal signal caught: ";
    constexpr char suffix[] = "\n";
    char buf[64];
    size_t len = 0;

    // copy prefix
    for (char c : prefix) {
      buf[len++] = c;
    }

    // convert signal number
    int_to_str(sig, buf, len);

    // copy suffix
    for (char c : suffix) {
      buf[len++] = c;
    }

    // write to stderr and exit
    [[maybe_unused]] ssize_t n = write(2, buf, len);
    _exit(1);
  }

  /// @brief Sets up global crash and exception handlers.
  ///
  /// Installs handlers for:
  /// - Unhandled exceptions (`std::terminate`)
  /// - Memory allocation failures (`std::bad_alloc`)
  /// - Fatal signals (SIGSEGV, SIGABRT, SIGFPE, SIGILL, and SIGBUS on POSIX)
  ///
  /// @note Signals like SIGKILL and SIGSTOP cannot be caught.
  static void configure_handlers() {
    // Catch unhandled exceptions
    std::set_terminate(handle_terminate);
    // Catch std::bad_alloc (new OOM)
    std::set_new_handler(handle_bad_alloc);
    // Catch common fatal signals
    std::signal(SIGSEGV, handle_signal);  // invalid memory access
    std::signal(SIGABRT, handle_signal);  // abort()
    std::signal(SIGFPE, handle_signal);   // divide by zero
    std::signal(SIGILL, handle_signal);   // illegal instruction
#ifdef __unix__
    std::signal(SIGBUS, handle_signal);  // bus error (POSIX)
#endif
    // SIGKILL and SIGSTOP cannot be caught
  }

 private:
  static inline void int_to_str(int n, char* buf, size_t& len) {
    if (n == 0) {
      buf[len++] = '0';
      return;
    }
    char tmp[16];
    int i = 0;
    while (n > 0 && i < 16) {
      tmp[i++] = '0' + (n % 10);
      n /= 10;
    }
    // reverse digits
    for (int j = i - 1; j >= 0; --j) {
      buf[len++] = tmp[j];
    }
  }
};

}  // namespace utils
