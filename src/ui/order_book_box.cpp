#include "order_book_box.h"

#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix44/Message.h>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "../binance/config.h"
#include "../binance/market_message_variant.h"
#include "../binance/symbol.h"
#include "../utils/double.h"
#include "../utils/threading.h"
#include "app/iscreen.h"
#include "helpers.h"
#include "spdlog/spdlog.h"

using ftxui::bold;
using ftxui::border;
using ftxui::Color;
using ftxui::Component;
using ftxui::dim;
using ftxui::Direction;
using ftxui::flex;
using ftxui::focusPositionRelative;
using ftxui::frame;
using ftxui::Renderer;
using ftxui::SliderOption;
using ftxui::text;
using ftxui::vbox;

using utils::Double;

namespace ui {

OrderBookBox::OrderBookBox(
    IScreen& screen,
    moodycamel::ConcurrentQueue<binance::MarketMessageVariant>& queue,
    const uint16_t MAX_DEPTH,
    core::OrderBook ob,
    std::function<void(std::stop_token)> task)
    : IS_BOOK_CLEAR_NEEDED_(MAX_DEPTH == 1),
      screen_(screen),
      core_book_(std::move(ob)),
      worker_task_(std::move(task)),
      queue_(queue) {
  // default behaviour
  if (!worker_task_) {
    worker_task_ = {[this](const std::stop_token& stoken) {
      utils::Threading::set_thread_name(THREAD_NAME_);
      spdlog::info("starting polling order queue on thread, name [{}], id [{}]",
                   THREAD_NAME_, utils::Threading::get_os_thread_id());
      poll_queue(stoken);
    }};
  }

  // initialize table header
  for (const auto& column : columns_) {
    header_.push_back(ftxui::text(Helpers::Pad(column.first, column.second)) |
                      ftxui::bold);
  }

  SliderOption<float> option_y;
  option_y.value = &scroll_y;
  option_y.min = 0.1f;
  option_y.max = 1.f;
  option_y.increment = 0.1f;
  option_y.direction = Direction::Down;
  option_y.color_active = Color::Yellow;
  option_y.color_inactive = Color::YellowLight;
  auto scrollbar_y = Slider(option_y);

  // Static header (always visible)
  auto header_renderer = Renderer([this] { return vbox({hbox(header_)}); });

  // Scrollable trade rows
  auto content = Renderer([this](bool focused) {
    return to_table() | (focused ? bold : dim) | focusPositionRelative(0, scroll_y) |
           frame | flex;
  });

  // Combine content and scrollbar horizontally
  auto scroll_area = ftxui::Container::Horizontal({content, scrollbar_y}) | flex;

  // Full component: static header + scrollable content
  component_ = ftxui::Container::Vertical({header_renderer, scroll_area}) | border;
}

void OrderBookBox::start() {
  // TODO (mils): thread_exception
  // start worker thread
  worker_ = std::jthread{worker_task_};
}

Component OrderBookBox::get_component() {
  return component_;
}

/// @brief generate an FTXUI table containing the order book
/// @return the FTXUI element that the UI will render
ftxui::Element OrderBookBox::to_table() {
  ftxui::Elements table;
  const std::vector<core::BidAsk> book = core_book_.to_vector();
  const size_t row_count = book.size();
  double bid_sz, bid_px, ask_px, ask_sz;
  for (size_t i = 0; i < row_count; ++i) {
    ftxui::Elements ui_row;
    const core::BidAsk& book_row = book[i];

    bid_sz = static_cast<double>(book_row.bid_sz) /
             binance::Config::get_size_ticks_per_unit(binance::SymbolEnum::BTCUSDT);
    bid_px = static_cast<double>(book_row.bid_px) /
             binance::Config::get_price_ticks_per_unit(binance::SymbolEnum::BTCUSDT);
    ask_px = static_cast<double>(book_row.ask_px) /
             binance::Config::get_price_ticks_per_unit(binance::SymbolEnum::BTCUSDT);
    ask_sz = static_cast<double>(book_row.ask_sz) /
             binance::Config::get_size_ticks_per_unit(binance::SymbolEnum::BTCUSDT);

    ui_row.push_back(
        ftxui::text(Helpers::Pad(utils::Double::trim(bid_sz), columns_[0].second)));
    ui_row.push_back(
        ftxui::text(Helpers::Pad(Double::pretty(bid_px), columns_[1].second)));
    ui_row.push_back(
        ftxui::text(Helpers::Pad(Double::pretty(ask_px), columns_[2].second)));
    ui_row.push_back(
        ftxui::text(Helpers::Pad(utils::Double::trim(ask_sz), columns_[3].second)));
    table.push_back(hbox(std::move(ui_row)));
  }

  return vbox(table);
};

// worker thread
void OrderBookBox::poll_queue(const std::stop_token& stoken) {
  try {
    /// Adaptive backoff strategy for spin+sleep polling
    /// Performs an adaptive backoff by spinning then sleeping, increasing sleep
    /// time exponentially. Yield 10x times, followed by a 2x sleep capped at
    /// 1ms
    constexpr uint8_t INITIAL_SLEEP_US = 10;
    uint8_t spin_count = 0;
    uint16_t sleep_time_us = INITIAL_SLEEP_US;
    auto adaptive_backoff = [&spin_count, &sleep_time_us]() {
      constexpr uint8_t MIN_SPINS = 10;
      constexpr uint16_t MAX_SLEEP_US = 1'000;
      if (spin_count < MIN_SPINS) {
        ++spin_count;
        std::this_thread::yield();
      } else {
        std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_us));
        sleep_time_us = std::min(static_cast<uint16_t>(sleep_time_us * 2u), MAX_SLEEP_US);
      }
    };

    while (!stoken.stop_requested()) {
      binance::MarketMessageVariant msg;
      while (!queue_.try_dequeue(msg)) {
        if (stoken.stop_requested()) {
          return;
        }
        adaptive_backoff();
      }

      std::visit(
          [&](auto& m) {
            using T = std::decay_t<decltype(m)>;
            if constexpr (std::is_same_v<T, FIX44::MarketDataSnapshotFullRefresh>) {
              core_book_.apply_snapshot(m);
            } else if constexpr (std::is_same_v<T, FIX44::MarketDataIncrementalRefresh>) {
              core_book_.apply_increment(m, IS_BOOK_CLEAR_NEEDED_);
            }
          },
          msg);

      screen_.post_event(ftxui::Event::Custom);
      // Reset backoff state
      spin_count = 0;
      sleep_time_us = INITIAL_SLEEP_US;
    }
    spdlog::info("closing worker thread, name [{}]", THREAD_NAME_);
  } catch (const std::exception& e) {
    spdlog::error("error in worker thread. name [{}], error [{}]", THREAD_NAME_,
                  e.what());
    thread_exception = std::current_exception();
  } catch (...) {
    spdlog::error("error in worker thread - unknown error. name [{}]", THREAD_NAME_);
    thread_exception = std::current_exception();
  }
}

}  // namespace ui
