#pragma once

#include <cstdint>
#include <string>
#include <thread>

namespace utils {

/// @brief thread helper functions
class Threading {
 public:
  /// @brief Returns the current thread's name. If unsupported, returns empty string
  static std::string get_thread_name();

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

  /// @brief Retrieves the number of logical CPU cores currently available to the process.
  /// On Linux and macOS, it queries `sysconf(_SC_NPROCESSORS_ONLN)` for an accurate,
  /// real-time count that respects CPU hotplug events and container (cgroup) limits.
  /// On Windows, it uses `GetSystemInfo()`. If platform-specific calls fail, the function
  /// falls back to `std::thread::hardware_concurrency()`.
  /// The returned value reflects the number of CPUs available to user-space threads,
  /// not necessarily the total number of physical cores. Hyperthreaded CPUs are counted
  /// as multiple logical processors.
  /// @return The number of available logical CPUs (>= 1). If detection fails, returns 1.
  /// @note CPU indices are zero-based, ranging from 0 to (get_cpu_count() - 1).
  static unsigned int get_cpu_count();

  /// @brief Sets the CPU affinity for the calling (current) thread.
  /// Attempts to pin the current thread to a specific logical CPU core,
  /// preserving cache locality and minimizing context switches.
  /// On Linux, this uses `pthread_setaffinity_np()` to bind the thread to the target CPU.
  /// On macOS, it applies a best-effort affinity hint via `thread_policy_set()`, since
  /// strict CPU pinning is not supported by the kernel.
  /// On Windows, it uses `SetThreadAffinityMask()` to achieve true CPU binding.
  /// @note On macOS, affinity is advisory only and may not guarantee the thread remains
  ///       on the specified CPU.
  /// @param cpu_id The zero-based index of the target logical CPU.
  ///               Valid range is `[0, get_cpu_count() - 1]`.
  static void set_thread_cpu(unsigned int cpu_id);

  /// @brief Elevates the current thread to real-time priority.
  /// Attempts to set the calling thread to the highest real-time priority:
  /// - On Windows: uses `SetThreadPriority()` with `THREAD_PRIORITY_TIME_CRITICAL`.
  /// - On POSIX: uses `pthread_setschedparam()` with `SCHED_FIFO` and priority 99.
  /// @note ! Requires root privileges on most POSIX systems
  static void set_thread_realtime();

 private:
  static void set_thread_affinity(std::thread& t, unsigned int cpu_id);
  // platform-specific affinity for native thread handle
  static void set_native_thread_affinity(std::thread::native_handle_type handle,
                                         unsigned int cpu_id);
};

}  // namespace utils
