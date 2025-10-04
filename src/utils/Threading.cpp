#include "Threading.h"

#include <string>
#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#elif defined(_WIN32)
#include <processthreadsapi.h>
#include <windows.h>
#endif

namespace utils {

constexpr int MAX_STRING_LENGTH = 15;

void Threading::set_thread_name(const std::string &name) {
#if defined(__linux__)
  // Linux: Limit is 16 bytes including null terminator
  pthread_setname_np(pthread_self(), name.substr(0, MAX_STRING_LENGTH).c_str());

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

}  // namespace utils
