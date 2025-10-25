#pragma once

#include <quickfix/fix44/Message.h>

#include <cstdint>
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
  static inline constexpr std::string THREAD_NAME_ = "ui_orderbook";
  // Constructor: takes a label string
  OrderBookBox(IScreen& screen,
               moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue,
               const uint16_t MAX_DEPTH,
               core::OrderBook ob = core::OrderBook{},
               std::function<void(std::stop_token)> task = {});
  // Return the FTXUI component to plug into layout
  ftxui::Component get_component();
  // if any exceptions occurred in the worker thread
  std::exception_ptr thread_exception;
  // start order processing worker thread
  void start();

 private:
  /// when working with Binance, if MAX_DEPTH is set to 1,
  /// price-update `FIX::MDUpdateAction_CHANGE` events need to clear the book.
  /// this boolean evaluates this condition.
  const bool IS_BOOK_CLEAR_NEEDED_;

  // ui
  IScreen& screen_;
  core::OrderBook core_book_;
  ftxui::Component component_;
  float scroll_y = 0;
  const std::array<std::pair<std::string, uint8_t>, 4> columns_ = {
      {{"Bid Sz", 10}, {"Bid", 13}, {"Ask", 13}, {"Ask Sz", 10}}};
  ftxui::Elements header_;

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
