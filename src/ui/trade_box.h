#pragma once

#include <quickfix/fix44/Message.h>

#include <deque>
#include <ftxui/component/screen_interactive.hpp>

#include "app/iscreen.h"
#include "concurrentqueue.h"

/*
step 1: vector of trades
step 2: sortable/searchable collection
step 3: fetch historical trades
*/

namespace ui {

class TradeBox {
 public:
  static constexpr std::string THREAD_NAME_ = "tradercppuiTX";
  TradeBox(IScreen& screen,
           moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue,
           std::function<void(std::stop_token)> task = {});
  // Return the FTXUI component to plug into layout
  ftxui::Component get_component();
  // if any exceptions occurred in the worker thread
  std::exception_ptr thread_exception;
  // start trade processing worker thread
  void start();

 private:
  // ui
  IScreen& screen_;
  ftxui::Component component_;
  float scroll_x = 0;
  float scroll_y = 0;

  // log ring-buffer
  std::deque<std::string> log_ring_;
  static constexpr int MAX_LINES_ = 100;

  // thread
  std::jthread worker_;
  std::function<void(std::stop_token)> worker_task_;
  // queue of order messages from FIX thread
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue_;
  /// @brief poll queue for any new FIX messages, update trade view
  /// @param stoken
  void poll_queue(const std::stop_token& stoken);
};

}  // namespace ui
