#pragma once

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <thread>

#include "../core/order_book.h"
#include "concurrentqueue.h"
#include "iscreen.h"

namespace ui {

class OrderBookBox {
 public:
  // Constructor: takes a label string
  OrderBookBox(IScreen& screen,
               moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue,
               std::unique_ptr<core::OrderBook> ob = std::make_unique<core::OrderBook>(),
               std::function<void(std::stop_token)> task = {});
  // Return the FTXUI component to plug into layout
  ftxui::Component get_component();
  /// if any exceptions occurred
  std::exception_ptr thread_exception;
  // start order processing worker thread
  void start();

 private:
  IScreen& screen_;
  std::unique_ptr<core::OrderBook> core_book_;
  ftxui::Component component_;

  // worker thread
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
