#include "threading.h"

#include <string>

#include "spdlog/spdlog.h"

#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#endif

#if defined(__linux__)
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>

#include <processthreadsapi.h>
#endif

namespace utils {

// static function
void Threading::set_thread_name(const std::string& name) {
#if defined(__linux__)
  // Linux: Limit is 16 bytes including null terminator
  pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());

#elif defined(__APPLE__)
  // macOS: Only supports setting name from within the thread
  pthread_setname_np(name.substr(0, 63).c_str());  // 64 bytes max including null

#elif defined(_WIN32)
  // Windows 10 1607 and later
  // Convert std::string to UTF-16 std::wstring
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
  if (size_needed > 0) {
    std::wstring wname(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, &wname[0], size_needed);

    // Set thread description
    HRESULT hr = SetThreadDescription(GetCurrentThread(), wname.c_str());
    if (FAILED(hr)) {
      // Optional: handle error (e.g., log or assert)
    }
  }
#endif
}

// static function
uint64_t Threading::get_os_thread_id() {
#if defined(__linux__)
  return static_cast<uint64_t>(::syscall(SYS_gettid));

#elif defined(__APPLE__)
  uint64_t tid;
  pthread_threadid_np(nullptr, &tid);
  return tid;

#elif defined(_WIN32)
  return static_cast<uint64_t>(::GetCurrentThreadId());

#else
  spdlog::error("unsupported platform, cannot get thread id. returning [{}]",
                ERROR_THREAD_ID);
  return ERROR_THREAD_ID;
#endif
}

}  // namespace utils
