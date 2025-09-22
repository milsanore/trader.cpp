#ifndef THREADINGUTILS_H
#define THREADINGUTILS_H

#include <string>

namespace Utils {

class Threading{
public:
    static void set_thread_name(const std::string& name);
};

}

#endif //THREADINGUTILS_H