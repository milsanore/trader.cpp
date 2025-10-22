#pragma once

#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <quickfix/fix44/Message.h>

#include <boost/circular_buffer.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <mutex>

#include "../binance/config.h"
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
           binance::Config& binance_config,
           moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue,
           std::function<void(std::stop_token)> task = {});
  // Return the FTXUI component to plug into layout
  ftxui::Component get_component();
  // if any exceptions occurred in the worker thread
  std::exception_ptr thread_exception;
  // start trade processing worker thread
  void start();
  //
  ftxui::Element to_table();

 private:
  // ui stuff
  IScreen& screen_;
  ftxui::Component component_;
  float scroll_y = 1;
  /// @brief the columns in the trade box table, and their widths
  const std::array<std::pair<std::string, uint8_t>, 5> columns_ = {
      {{"Time", 17}, {"Side", 6}, {"Price", 13}, {"Size", 10}, {"ID", 13}}};
  ftxui::Elements header_;
  binance::Config& binance_config_;

  // trade ring-buffer stuff
  static constexpr u_int16_t MAX_LINES_ = 100;
  boost::circular_buffer<core::Trade> trade_ring_;
  std::mutex trade_ring_mutex_;

  // worker thread stuff
  // queue of order messages from FIX thread
  moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue_;
  std::jthread worker_;
  std::function<void(std::stop_token)> worker_task_;
  /// @brief poll queue for any new FIX messages, trigger UI render.
  /// runs on worker thread ( @ref ui::TradeBox::THREAD_NAME_ )
  void poll_queue(const std::stop_token& stoken);
  /// @brief add new trades to ring buffer
  /// runs on worker thread ( @ref ui::TradeBox::THREAD_NAME_ )
  void on_trade(const FIX44::MarketDataIncrementalRefresh& msg);
};

}  // namespace ui
