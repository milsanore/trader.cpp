#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>

#if defined(__GNUG__)
#include <cxxabi.h>

#include <cstdlib>
#endif

namespace utils {

class Types {
  using malloc_ptr = std::unique_ptr<char, void (*)(void*)>;

 public:
  /// @brief Get the readable name of an *instance* of a type
  template <typename T>
  static std::string type_name(const T& var) {
    return type_name<T>();
  }

  /// @brief Get the readable name of a *type*
  template <typename T>
  static std::string type_name() {
    const char* mangled = typeid(T).name();
    return unmangle(mangled);
  }

 private:
  static std::string unmangle(const char* mangled_string) {
#if defined(__GNUG__)
    int status = 0;
    malloc_ptr demangled(abi::__cxa_demangle(mangled_string, nullptr, nullptr, &status),
                         std::free);
    return (status == 0 && demangled) ? demangled.get() : mangled_string;
#else
    // On MSVC or other compilers, just return the mangled name
    // (usually already readable)
    return mangled;
#endif
  }
};

}  // namespace utils
