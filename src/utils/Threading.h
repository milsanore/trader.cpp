#pragma once

#include <string>

namespace Utils {

class Threading {
 public:
  static void set_thread_name(const std::string &name);
};

}  // namespace Utils
