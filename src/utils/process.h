#include <stdio.h>
#include <stdlib.h>

#include <format>
#include <stdexcept>

#include "spdlog/spdlog.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace utils {

/// @brief application process helpers
struct Process {
 public:
  /// @brief Sets the current process to run with high priority.
  ///
  /// This function attempts to increase the execution priority of the current process.
  /// On Windows systems, it sets the process priority class to a higher level
  /// (e.g. `HIGH_PRIORITY_CLASS`). On Unix-like systems, it adjusts the process "nice"
  /// value. The function uses `utils::Process::set_process_priority()` to apply the
  /// change and logs the result.
  ///
  /// @throws std::runtime_error If the process priority cannot be changed successfully.
  static void set_high_priority() {
    int priority = 0;
#if defined(_WIN32) || defined(_WIN64)
    priority = 1;  // HIGH_PRIORITY_CLASS
#else
    priority = -20;  // Unix nice value
#endif
    utils::Process::set_process_priority(priority);
  }

 private:
  /// @brief Set the priority of the current process.
  /// @param priority  On Linux/macOS: nice value (-20 highest → +19 lowest)
  ///                  On Windows: 0=NORMAL, 1=HIGH, 2=REALTIME
  /// @throw std::runtime_error if priority cannot be set
  static void set_process_priority(int priority) {
#if defined(_WIN32) || defined(_WIN64)
    DWORD win_prio;
    switch (priority) {
      case 0:
        win_prio = NORMAL_PRIORITY_CLASS;
        break;
      case 1:
        win_prio = HIGH_PRIORITY_CLASS;
        break;
      case 2:
        win_prio = REALTIME_PRIORITY_CLASS;
        break;
      default:
        win_prio = NORMAL_PRIORITY_CLASS;
        break;
    }
    if (!SetPriorityClass(GetCurrentProcess(), win_prio)) {
      throw std::runtime_error("failed to set windows process priority");
    }

#else
    int ret = setpriority(PRIO_PROCESS, 0, priority);
    if (ret != 0) {
      perror("setpriority");
      throw std::runtime_error(std::format(
          "failed to set posix process priority. value [{}], ret [{}]", priority, ret));
    }
#endif
  }
};

}  // namespace utils
