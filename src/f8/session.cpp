#include "session.h"

#include <functional>
#include <map>

namespace f8 {

bool Session::handle_application(const unsigned /*seqnum*/,
                                 const FIX8::Message*& /*msg*/) {
  //   return enforce(seqnum, msg) || msg->process(_router);
  return true;
}

}  // namespace f8
