#include "order_book_box.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "./../utils/threading.h"
#include "app/iscreen.h"
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

namespace ui {

OrderBookBox::OrderBookBox(
    IScreen& screen,
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue,
    const int MAX_DEPTH,
    core::OrderBook ob,
    std::function<void(std::stop_token)> task)
    : IS_BOOK_CLEAR_NEEDED_(MAX_DEPTH == 1),
      screen_(screen),
      queue_(queue),
      core_book_(std::move(ob)),
      worker_task_(std::move(task)) {
  // default behaviour
  if (!worker_task_) {
    worker_task_ = {[this](const std::stop_token& stoken) {
      utils::Threading::set_thread_name(THREAD_NAME_);
      spdlog::info("starting polling order queue on thread, name [{}], id [{}]",
                   THREAD_NAME_, utils::Threading::get_os_thread_id());
      poll_queue(stoken);
    }};
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

  auto content = Renderer([this](bool focused) {
    return to_table() | (focused ? bold : dim) | focusPositionRelative(0, scroll_y) |
           frame | flex | border;
  });

  component_ = ftxui::Container::Horizontal({content, scrollbar_y}) | flex;
}

void OrderBookBox::start() {
  // TODO (mils): thread_exception
  // start worker thread
  worker_ = std::jthread{worker_task_};
}

Component OrderBookBox::get_component() {
  return component_;
}

// Helper to pad or truncate a string to a fixed width
std::string Pad(const std::string& input, const size_t width) {
  if (input.size() >= width) {
    return input.substr(0, width);
  }

  std::string result = input;
  result.resize(width, ' ');
  return result;
}
std::string Pad(const double input, const size_t width) {
  return Pad(std::isnan(input) ? "" : std::to_string(input), width);
}

/// @brief generate an FTXUI table containing the order book
/// @return the FTXUI element that the UI will render
ftxui::Element OrderBookBox::to_table() {
  ftxui::Elements table;

  // ─────────── Header Row ───────────
  const std::vector<std::string> column_names = {"Bid Sz", "Bid", "Ask", "Ask Sz"};
  constexpr int column_width = 15;
  constexpr int num_columns = 4;
  ftxui::Elements header;
  for (int i = 0; i < num_columns; ++i) {
    header.push_back(ftxui::text(Pad(column_names[i], column_width)) | ftxui::bold);
  }
  table.push_back(ftxui::vbox({hbox(std::move(header)), ftxui::separator()}));

  // ─────────── Data Rows ───────────
  const std::vector<core::BidAsk> book = core_book_.to_vector();
  const size_t row_count = book.size();
  for (size_t i = 0; i < row_count; ++i) {
    ftxui::Elements ui_row;
    const core::BidAsk& book_row = book[i];
    ui_row.push_back(ftxui::text(Pad(book_row.bid_sz, column_width)));
    ui_row.push_back(ftxui::text(Pad(book_row.bid_px, column_width)));
    ui_row.push_back(ftxui::text(Pad(book_row.ask_px, column_width)));
    ui_row.push_back(ftxui::text(Pad(book_row.ask_sz, column_width)));
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
    constexpr int INITIAL_SLEEP_US = 10;
    int spin_count = 0;
    int sleep_time_us = INITIAL_SLEEP_US;
    auto adaptive_backoff = [&spin_count, &sleep_time_us]() {
      constexpr int MIN_SPINS = 10;
      constexpr int MAX_SLEEP_US = 1000;
      if (spin_count < MIN_SPINS) {
        ++spin_count;
        std::this_thread::yield();
      } else {
        std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_us));
        sleep_time_us = std::min(sleep_time_us * 2, MAX_SLEEP_US);
      }
    };

    while (!stoken.stop_requested()) {
      std::shared_ptr<const FIX44::Message> msg;
      while (!queue_.try_dequeue(msg)) {
        if (stoken.stop_requested()) {
          return;
        }
        adaptive_backoff();
      }

      if (auto snap =
              std::dynamic_pointer_cast<const FIX44::MarketDataSnapshotFullRefresh>(
                  msg)) {
        // TODO (mils): wrap each update in a try/catch?
        core_book_.apply_snapshot(*snap);
        screen_.post_event(ftxui::Event::Custom);  // Trigger re-render
      } else if (auto inc =
                     std::dynamic_pointer_cast<const FIX44::MarketDataIncrementalRefresh>(
                         msg)) {
        // TODO (mils): wrap each update in a try/catch?
        core_book_.apply_increment(*inc, IS_BOOK_CLEAR_NEEDED_);
        screen_.post_event(ftxui::Event::Custom);  // Trigger re-render
      } else {
        spdlog::error("unknown message type");
      }

      // Reset backoff state
      spin_count = 0;
      sleep_time_us = INITIAL_SLEEP_US;
    }
    spdlog::info("closing worker thread...");
  }
  // TODO(mils): log error
  catch (const std::exception& e) {
    spdlog::error("error in orderbook worker thread. error [{}]", e.what());
    thread_exception = std::current_exception();
  } catch (...) {
    spdlog::error("error in orderbook worker thread. unknown error");
    thread_exception = std::current_exception();
  }
}

}  // namespace ui
