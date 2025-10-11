#include "trade_box.h"

#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <quickfix/fix44/Message.h>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "./../utils/threading.h"
#include "concurrentqueue.h"
#include "spdlog/spdlog.h"

using ftxui::bold;
using ftxui::border;
using ftxui::Component;
using ftxui::dim;
using ftxui::Renderer;
using ftxui::text;
using ftxui::vbox;

namespace ui {

TradeBox::TradeBox(
    IScreen& screen,
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue,
    std::function<void(std::stop_token)> task)
    : screen_(screen), queue_(queue), worker_task_(task) {
  // default behaviour
  if (!worker_task_) {
    worker_task_ = {[this](const std::stop_token& stoken) {
      utils::Threading::set_thread_name(THREAD_NAME_);
      spdlog::info("starting polling trade queue on thread, name [{}], id [{}]",
                   THREAD_NAME_, utils::Threading::get_os_thread_id());
      poll_queue(stoken);
    }};
  }

  component_ = Renderer([this](bool focused) {
    auto body = text(focused ? "Focused" : "Not Focused");
    return vbox(body) | border | (focused ? bold : dim);
  });
}

void TradeBox::start() {
  // TODO (mils): thread_exception
  // start worker thread
  worker_ = std::jthread{worker_task_};
}

Component TradeBox::get_component() {
  return component_;
}

// worker thread
void TradeBox::poll_queue(const std::stop_token& stoken) {
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

      if (auto inc =
              std::dynamic_pointer_cast<const FIX44::MarketDataIncrementalRefresh>(msg)) {
        // TODO (mils): wrap each update in a try/catch?
        // core_book_.apply_increment(*inc, IS_BOOK_CLEAR_NEEDED_);
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

}  // namespace ui
