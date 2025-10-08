#pragma once

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <thread>

#include "../core/order_book.h"
#include "app/iscreen.h"
#include "concurrentqueue.h"

namespace ui {

class OrderBookBox {
 public:
  static constexpr std::string thread_name_ = "tradercppuiBOOK";
  // Constructor: takes a label string
  OrderBookBox(IScreen& screen,
               moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue,
               core::OrderBook ob = core::OrderBook(),
               std::function<void(std::stop_token)> task = {});
  // Return the FTXUI component to plug into layout
  ftxui::Component get_component();
  /// if any exceptions occurred
  std::exception_ptr thread_exception;
  // start order processing worker thread
  void start();

 private:
  IScreen& screen_;
  core::OrderBook core_book_;
  ftxui::Component component_;
  float scroll_x = 0;
  float scroll_y = 0;

  // thread
  std::jthread worker_;
  std::function<void(std::stop_token)> worker_task_;
  // queue of order messages from FIX thread
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue_;
  /// @brief poll queue for any new FIX messages, update order book
  /// @param stoken
  void poll_queue(const std::stop_token& stoken);
  /// @brief
  /// @return
  ftxui::Element to_table();
};

}  // namespace ui
