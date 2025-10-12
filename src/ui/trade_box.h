#pragma once

#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <quickfix/fix44/Message.h>

#include <boost/circular_buffer.hpp>
#include <deque>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <mutex>

#include "../core/trade.h"
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
  float scroll_y = 1;
  ftxui::Element to_table();

  // trade ring-buffer
  static constexpr u_int16_t MAX_LINES_ = 100;
  boost::circular_buffer<core::Trade> trade_ring_;
  std::mutex trade_ring_mutex_;

  // thread
  std::jthread worker_;
  std::function<void(std::stop_token)> worker_task_;
  // queue of order messages from FIX thread
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue_;
  /// @brief poll queue for any new FIX messages, update trade view
  void poll_queue(const std::stop_token& stoken);
  void on_trade(const FIX44::MarketDataIncrementalRefresh& msg);
};

}  // namespace ui
