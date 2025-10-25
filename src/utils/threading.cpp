
#include "threading.h"

#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>
#include <thread>

#include "spdlog/spdlog.h"

#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#endif

#if defined(__linux__)
#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <mach/thread_policy.h>
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
std::string Threading::get_thread_name() {
#if defined(__linux__)
  char name[16] = {0};  // Linux max length for pthread_getname_np
  int rc = pthread_getname_np(pthread_self(), name, sizeof(name));
  if (rc != 0) {
    spdlog::error("failed to get thread name. error [{}]",
                  std::system_category().message(rc));
    return {};
  }
  return std::string(name);

#elif defined(__APPLE__)
  char name[64] = {0};  // macOS max 64 including null terminator
  int rc = pthread_getname_np(pthread_self(), name, sizeof(name));
  if (rc != 0) {
    spdlog::error("failed to get thread name on macOS. error [{}]", rc);
    return {};
  }
  return std::string(name);

#elif defined(_WIN32)
  // Windows 10 1607+ has GetThreadDescription
  PWSTR wname = nullptr;
  HRESULT hr = GetThreadDescription(GetCurrentThread(), &wname);
  if (FAILED(hr) || wname == nullptr) {
    return {};
  }
  // Convert UTF-16 to UTF-8
  int size_needed =
      WideCharToMultiByte(CP_UTF8, 0, wname, -1, nullptr, 0, nullptr, nullptr);
  std::string name(size_needed, 0);
  WideCharToMultiByte(CP_UTF8, 0, wname, -1, &name[0], size_needed, nullptr, nullptr);
  LocalFree(wname);  // free memory allocated by GetThreadDescription
  // remove trailing null if present
  if (!name.empty() && name.back() == '\0') {
    name.pop_back();
  }
  return name;

#else
  return {};  // Unsupported platform
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

// static function
unsigned int Threading::get_cpu_count() {
#if defined(__linux__) || defined(__APPLE__)
  long n = ::sysconf(_SC_NPROCESSORS_ONLN);
  if (n > 0) {
    return static_cast<unsigned int>(n);
  }
  // Fallback if sysconf fails
  unsigned int std_n = std::thread::hardware_concurrency();
  return std_n > 0 ? std_n : 1;

#elif defined(_WIN32)
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;

#else
  unsigned int std_n = std::thread::hardware_concurrency();
  return std_n > 0 ? std_n : 1;
#endif
}

// static function
bool Threading::set_native_thread_affinity(std::thread::native_handle_type handle,
                                           unsigned int cpu_id) {
#if defined(__linux__)
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_id, &cpuset);
  unsigned int num_cpus = get_cpu_count();
  if (cpu_id >= num_cpus) {
    spdlog::error("cpu_id exceeds available CPUs, cpu_id [{}] available [{}]", cpu_id,
                  num_cpus);
    return false;
  }
  int rc = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset);
  if (rc != 0) {
    spdlog::error("linux: failed to set thread affinity. error [{}]",
                  std::system_category().message(rc));
    return false;
  }
  spdlog::info("linux: successfully set thread affinity. thread_name [{}] cpu [{}]",
               get_thread_name(), cpu_id);
  return true;

#elif defined(__APPLE__)
  thread_port_t mach_thread = pthread_mach_thread_np(handle);
  thread_affinity_policy_data_t policy{.affinity_tag = static_cast<int>(cpu_id + 1)};
  kern_return_t kr = thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY,
                                       reinterpret_cast<thread_policy_t>(&policy),
                                       THREAD_AFFINITY_POLICY_COUNT);
  if (kr != KERN_SUCCESS) {
    spdlog::error("macOS: failed to set thread affinity. kern_return_t [{}]", kr);
    return false;
  }
  spdlog::info("macOS: successfully set thread affinity. thread_name [{}] cpu [{}]",
               get_thread_name(), cpu_id);
  return true;

#elif defined(_WIN32)
  DWORD_PTR mask = static_cast<DWORD_PTR>(1) << cpu_id;
  DWORD_PTR result = SetThreadAffinityMask(handle, mask);
  if (result == 0) {
    spdlog::error("windows: failed to set thread affinity. error [{}]", GetLastError());
    return false;
  }
  spdlog::info("windows: successfully set thread affinity. thread_name [{}] cpu [{}]",
               get_thread_name(), cpu_id);
  return true;

#else
  (void)handle;
  (void)cpu_id;
  spdlog::error("thread affinity not supported on this platform");
  return false;
#endif
}

// static function
bool Threading::set_thread_affinity(std::thread& t, unsigned int cpu_id) {
  return set_native_thread_affinity(t.native_handle(), cpu_id);
}

// static function
bool Threading::set_current_thread_affinity(unsigned int cpu_id) {
#if defined(__linux__) || defined(__APPLE__)
  return set_native_thread_affinity(pthread_self(), cpu_id);
#elif defined(_WIN32)
  return set_native_thread_affinity(GetCurrentThread(), cpu_id);
#else
  spdlog::error("thread affinity not supported on this platform");
  return false;
#endif
}

}  // namespace utils
