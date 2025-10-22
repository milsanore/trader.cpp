#include <functional>
#include <thread>

namespace utils {

class Testing {
 public:
  /// @brief Repeatedly evaluates a condition until it returns true or a timeout is
  /// reached. This utility is useful for waiting on asynchronous operations in tests,
  /// such as waiting for background threads or UI updates.
  ///
  /// @param condition A callable that returns true when the desired state is reached.
  /// @param timeout_ms The maximum time to wait (in milliseconds) before giving up.
  /// @param interval_ms How often to poll the condition (in milliseconds). Defaults to
  /// 50ms.
  /// @return true if the condition was met within the timeout, false otherwise.
  static bool wait_for(std::function<bool()> condition,
                       int timeout_ms,
                       int interval_ms = 50) {
    int waited = 0;
    while (waited < timeout_ms) {
      if (condition()) {
        return true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
      waited += interval_ms;
    }
    return false;
  }
};

}  // namespace utils
