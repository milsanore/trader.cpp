#include "trade_box.h"

#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <quickfix/fix44/Message.h>

#include <cstdint>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <mutex>

#include "../binance/config.h"
#include "../binance/side.h"
#include "../binance/symbol.h"
#include "../core/trade.h"
#include "../utils/double.h"
#include "../utils/threading.h"
#include "concurrentqueue.h"
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

namespace ui {

TradeBox::TradeBox(
    IScreen& screen,
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue,
    std::function<void(std::stop_token)> task)
    : screen_(screen), trade_ring_(MAX_LINES_), queue_(queue), worker_task_(task) {
  // default behaviour
  if (!worker_task_) {
    worker_task_ = {[this](const std::stop_token& stoken) {
      utils::Threading::set_thread_name(THREAD_NAME_);
      spdlog::info("starting polling trade queue on thread, name [{}], id [{}]",
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

void TradeBox::start() {
  // TODO (mils): thread_exception
  // start worker thread
  worker_ = std::jthread{worker_task_};
}

Component TradeBox::get_component() {
  return component_;
}

ftxui::Element TradeBox::to_table() {
  boost::circular_buffer<core::Trade> buffer_copy;
  {
    std::lock_guard lock(trade_ring_mutex_);
    buffer_copy = trade_ring_;
  }

  ftxui::Elements table;

  // ─────────── Data Rows ───────────
  const size_t buffer_size = buffer_copy.size();
  double trade_sz, trade_px;
  for (size_t i = 0; i < buffer_size; ++i) {
    ftxui::Elements ui_row;
    const core::Trade& trade = buffer_copy[i];
    std::string side = binance::Side::to_str(trade.side);
    trade_sz = static_cast<double>(trade.sz) /
               binance::Config::get_size_ticks_per_unit(binance::SymbolEnum::BTCUSDT);
    trade_px = static_cast<double>(trade.px) /
               binance::Config::get_price_ticks_per_unit(binance::SymbolEnum::BTCUSDT);
    ui_row.push_back(ftxui::text(Helpers::Pad(trade.time, columns_[0].second)));
    ui_row.push_back(ftxui::text(Helpers::Pad(side, columns_[1].second)));
    ui_row.push_back(
        ftxui::text(Helpers::Pad(utils::Double::pretty(trade_px), columns_[2].second)));
    ui_row.push_back(
        ftxui::text(Helpers::Pad(utils::Double::trim(trade_sz), columns_[3].second)));
    ui_row.push_back(ftxui::text(Helpers::Pad(trade.id, columns_[4].second)));
    table.push_back(hbox(std::move(ui_row)));
  }

  return vbox(table);
}

void TradeBox::poll_queue(const std::stop_token& stoken) {
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
      std::shared_ptr<const FIX44::Message> msg;
      while (!queue_.try_dequeue(msg)) {
        if (stoken.stop_requested()) {
          return;
        }
        adaptive_backoff();
      }

      if (auto inc =
              std::dynamic_pointer_cast<const FIX44::MarketDataIncrementalRefresh>(msg)) {
        // TODO (mils): wrap each update in a try/catch?
        on_trade(*inc);
        screen_.post_event(ftxui::Event::Custom);
      } else {
        spdlog::error(
            "cannot parse trade view update - unknown message type. message [{}]",
            msg->toString());
      }

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

void TradeBox::on_trade(const FIX44::MarketDataIncrementalRefresh& msg) {
  std::lock_guard lock(trade_ring_mutex_);
  FIX::NoMDEntries entries;
  msg.get(entries);
  const int num_entries = entries.getValue();

  FIX44::MarketDataIncrementalRefresh::NoMDEntries group;
  FIX::Symbol fsym;
  std::optional<binance::SymbolEnum> symbol;
  FIX::MDEntryPx e_px;
  FIX::MDEntrySize e_sz;
  uint64_t price, size;
  FIX::MDEntryType e_type;
  FIX::TransactTime time;
  FIX::TradeID trade_id;
  std::string trade_id_str;
  uint64_t trade_id_uint;
  // Binance's "side" field ("AggressorSide") is a custom field, not part of the FIX spec
  constexpr int AGGRESSOR_TAG = 2446;
  FIX::CharField side_field(AGGRESSOR_TAG);
  binance::SideEnum side{};
  //
  std::string raw_time;
  std::string time_only;
  for (int i = 1; i <= num_entries; i++) {
    trade_id_str.clear();
    raw_time.clear();
    time_only.clear();
    trade_id_uint = 0;

    msg.getGroup(i, group);

    // Update symbol if present or first group
    if (i == 1 || group.isSetField(FIX::FIELD::Symbol)) {
      group.get(fsym);
      symbol = binance::Symbol::from_str(fsym.getValue());
    }
    if (!symbol) {
      spdlog::error("missing symbol, skipping trade entry. value [{}]");
      continue;
    }
    // debug
    if (symbol != binance::SymbolEnum::BTCUSDT) {
      spdlog::error("wrong symbol, skipping increment. value [{}]",
                    binance::Symbol::to_str(symbol.value()));
      continue;
    }

    group.get(e_type);
    switch (e_type.getValue()) {
      case FIX::MDEntryType_TRADE: {
        if (group.isSetField(AGGRESSOR_TAG)) {
          group.getField(side_field);
          side = binance::Side::from_str(side_field.getValue());
          price = utils::Double::toUint64(
              group.get(e_px).getValue(),
              binance::Config::get_price_ticks_per_unit(symbol.value()));
          size = utils::Double::toUint64(
              group.get(e_sz).getValue(),
              binance::Config::get_size_ticks_per_unit(symbol.value()));
          group.getField(trade_id);
          trade_id_str = trade_id.getValue();
          std::string timeOnly = "";
          if (group.isSetField(FIX::FIELD::TransactTime)) {
            FIX::TransactTime transactTime;
            group.getField(transactTime);
            raw_time = transactTime.getString();
            time_only = raw_time.substr(raw_time.find('-') + 1);
          }
          std::from_chars(trade_id_str.data(), trade_id_str.data() + trade_id_str.size(),
                          trade_id_uint);

          trade_ring_.push_back({time_only, side, price, size, trade_id_uint});
          screen_.post_event(ftxui::Event::Custom);
        } else {
          spdlog::error("trade with no side. message [{}]", msg.toString());
        }
      } break;
      default:
        spdlog::error("unknown trade FIX::MDEntryType. value [{}]", e_type.getValue());
    }
  }
}

}  // namespace ui
