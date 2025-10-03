#include "OrderBookBox.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "./../utils/Threading.h"
#include "IScreen.h"
#include "spdlog/spdlog.h"

using namespace ftxui;

namespace UI {

OrderBookBox::OrderBookBox(
    IScreen &screen,
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &queue,
    std::unique_ptr<Core::OrderBook> ob, std::function<void(std::stop_token)> task)
    : screen_(screen),
      queue_(queue),
      coreBook_(std::move(ob)),
      workerTask_(std::move(task)) {
  // default behaviour
  if (!workerTask_) {
    workerTask_ = ([this](const std::stop_token &stoken) {
      Utils::Threading::set_thread_name("tradercppuiBOOK");
      spdlog::info("starting polling order queue on background thread");
      pollQueue(stoken);
    });
  }

  component_ = Renderer(
      [this](bool focused) { return toTable() | border | (focused ? bold : dim); });
}

void OrderBookBox::Start() {
  // start worker thread
  worker_ = std::jthread(workerTask_);
}

Component OrderBookBox::GetComponent() { return component_; }

// Helper to pad or truncate a string to a fixed width
std::string Pad(const std::string &input, const size_t width) {
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
ftxui::Element OrderBookBox::toTable() {
  // NB: runs on main thread
  ftxui::Elements table_elements;
  constexpr int num_columns = 4;
  constexpr int column_width = 15;
  const std::vector<std::string> column_headers = {"Bid Sz", "Bid", "Ask", "Ask Sz"};

  // ─────────── Header Row ───────────
  ftxui::Elements header_cells;
  for (int col = 0; col < num_columns; ++col) {
    std::string label = (col < column_headers.size()) ? column_headers[col]
                                                      : "Col " + std::to_string(col);
    header_cells.push_back(ftxui::text(Pad(label, column_width)) | ftxui::bold);
  }

  // Combine header row and separator
  table_elements.push_back(
      ftxui::vbox({hbox(std::move(header_cells)), ftxui::separator()}));

  // ─────────── Data Rows ───────────
  const std::vector<Core::BidAsk> x = coreBook_->toVector();
  constexpr size_t max_rows = 15;
  const size_t rowCount = std::min(x.size(), max_rows);
  for (size_t i = 0; i < rowCount; ++i) {
    ftxui::Elements cells;
    cells.push_back(ftxui::text(Pad(x[i].bid_sz, column_width)));
    cells.push_back(ftxui::text(Pad(x[i].bid_px, column_width)));
    cells.push_back(ftxui::text(Pad(x[i].ask_px, column_width)));
    cells.push_back(ftxui::text(Pad(x[i].ask_sz, column_width)));
    table_elements.push_back(hbox(std::move(cells)));
  }
  if (rowCount >= max_rows) {
    ftxui::Elements cells;
    cells.push_back(ftxui::text(Pad("...", column_width)));
    cells.push_back(ftxui::text(Pad("...", column_width)));
    cells.push_back(ftxui::text(Pad("...", column_width)));
    cells.push_back(ftxui::text(Pad("...", column_width)));
    table_elements.push_back(hbox(std::move(cells)));
  }

  return vbox(table_elements);
};

// worker thread
void OrderBookBox::pollQueue(const std::stop_token &stoken) {
  try {
    /// Adaptive backoff strategy for spin+sleep polling
    /// Performs an adaptive backoff by spinning then sleeping, increasing sleep
    /// time exponentially. Yield 10x times, followed by a 2x sleep capped at
    /// 1ms
    constexpr int INITIAL_SLEEP_US = 10;
    int spinCount = 0;
    int sleepTimeUs = INITIAL_SLEEP_US;
    auto adaptiveBackoff = [&spinCount, &sleepTimeUs]() {
      constexpr int MIN_SPINS = 10;
      constexpr int MAX_SLEEP_US = 1000;
      if (spinCount < MIN_SPINS) {
        ++spinCount;
        std::this_thread::yield();
      } else {
        std::this_thread::sleep_for(std::chrono::microseconds(sleepTimeUs));
        sleepTimeUs = std::min(sleepTimeUs * 2, MAX_SLEEP_US);
      }
    };

    Core::OrderBook &book = *coreBook_;
    while (!stoken.stop_requested()) {
      std::shared_ptr<const FIX44::Message> msg;
      while (!queue_.try_dequeue(msg)) {
        if (stoken.stop_requested()) {
          return;
        }
        adaptiveBackoff();
      }

      if (auto snap =
              std::dynamic_pointer_cast<const FIX44::MarketDataSnapshotFullRefresh>(
                  msg)) {
        book.applySnapshot(*snap);
        screen_.PostEvent(ftxui::Event::Custom);  // Trigger re-render
      } else if (auto inc =
                     std::dynamic_pointer_cast<const FIX44::MarketDataIncrementalRefresh>(
                         msg)) {
        book.applyIncrement(*inc);
        screen_.PostEvent(ftxui::Event::Custom);  // Trigger re-render
      } else {
        spdlog::error("unknown message type");
      }

      // Reset backoff state
      spinCount = 0;
      sleepTimeUs = INITIAL_SLEEP_US;
    }
    spdlog::info("closing worker thread...");
  }
  // TODO(mils): log error
  catch (const std::exception &e) {
    spdlog::error("error in orderbook worker thread, error [{}]", e.what());
    thread_exception = std::current_exception();
  } catch (...) {
    spdlog::error(std::format("error in orderbook worker thread, unknown error"));
    thread_exception = std::current_exception();
  }
}

}  // namespace UI
