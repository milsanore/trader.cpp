#ifndef UTILS_THREADING_H
#define UTILS_THREADING_H

#include <string>

namespace Utils {

class Threading {
 public:
  static void set_thread_name(const std::string &name);
};

}  // namespace Utils

#endif  // UTILS_THREADING_H
