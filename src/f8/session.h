#pragma once

#include <functional>
#include <map>

#include "includes.h"
#include "router.h"

namespace f8 {

class Session : public FIX8::Session {
  Router _router;

 public:
  Session(const FIX8::F8MetaCntx& ctx,
          const FIX8::SessionID& sid,
          FIX8::Persister* persist = nullptr,
          FIX8::Logger* logger = nullptr,
          FIX8::Logger* plogger = nullptr)
      : FIX8::Session(ctx, sid, persist, logger, plogger), _router(*this) {}

  // Override these methods if required but remember to call the base class method first.
  // bool handle_logon(const unsigned seqnum, const FIX8::Message *msg);
  // Message *generate_logon(const unsigned heartbeat_interval, const f8String
  // davi=f8String()); bool handle_logout(const unsigned seqnum, const FIX8::Message
  // *msg); Message *generate_logout(); bool handle_heartbeat(const unsigned seqnum, const
  // FIX8::Message *msg); Message *generate_heartbeat(const f8String& testReqID); bool
  // handle_resend_request(const unsigned seqnum, const FIX8::Message *msg); Message
  // *generate_resend_request(const unsigned begin, const unsigned end=0); bool
  // handle_sequence_reset(const unsigned seqnum, const FIX8::Message *msg); Message
  // *generate_sequence_reset(const unsigned newseqnum, const bool gapfillflag=false);
  // bool handle_test_request(const unsigned seqnum, const FIX8::Message *msg);
  // Message *generate_test_request(const f8String& testReqID);
  // bool handle_reject(const unsigned seqnum, const FIX8::Message *msg);
  // Message *generate_reject(const unsigned seqnum, const char *what);
  // bool handle_admin(const unsigned seqnum, const FIX8::Message *msg);
  // void modify_outbound(FIX8::Message *msg);
  // bool authenticate(SessionID& id, const FIX8::Message *msg);

  // Override these methods to intercept admin and application methods.
  // bool handle_admin(const unsigned seqnum, const FIX8::Message *msg);

  bool handle_application(const unsigned seqnum, const FIX8::Message*& msg) override;
};

}  // namespace f8
