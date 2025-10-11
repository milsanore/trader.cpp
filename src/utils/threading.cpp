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
  pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());

#elif defined(__APPLE__)
  pthread_setname_np(name.substr(0, 63).c_str());

#elif defined(_WIN32)
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
  if (size_needed > 0) {
    std::wstring wname(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, &wname[0], size_needed);
    SetThreadDescription(GetCurrentThread(), wname.c_str());
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
