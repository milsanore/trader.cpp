#pragma once

#include <string>

namespace utils {

class Threading {
 public:
  static void set_thread_name(const std::string &name);
};

}  // namespace utils
