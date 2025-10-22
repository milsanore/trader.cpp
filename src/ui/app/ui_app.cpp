#include "ui_app.h"

#include <quickfix/fix44/Message.h>

#include <algorithm>
#include <cstdint>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "../../binance/config.h"
#include "../log_box/log_box.h"
#include "../order_book_box.h"
#include "../trade_box.h"
#include "../wallet_box.h"
#include "concurrentqueue.h"
#include "ftxui_screen.h"
#include "iscreen.h"
#include "spdlog/spdlog.h"

using ftxui::EQUAL;
using ftxui::flex;
using ftxui::HEIGHT;
using ftxui::size;
using ftxui::WIDTH;
using ftxui::Container::Horizontal;
using ftxui::Container::Vertical;

namespace ui {

///////////////////////////////////////

App::App(std::unique_ptr<IScreen> screen,
         std::unique_ptr<OrderBookBox> book_box,
         std::unique_ptr<LogBox> log_box,
         std::unique_ptr<TradeBox> trade_box)
    : screen_(std::move(screen)),
      book_box_(std::move(book_box)),
      log_box_(std::move(log_box)),
      trade_box_(std::move(trade_box)) {};

// static function
App App::from_env(
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& order_queue,
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& trade_queue,
    binance::Config& binance_config) {
  //
  std::unique_ptr<IScreen> screen = std::make_unique<FtxuiScreen>();

  auto book_box =
      std::make_unique<OrderBookBox>(*screen, order_queue, binance_config.MAX_DEPTH);

  auto log_box = LogBox::from_env(*screen);

  auto trade_box = std::make_unique<TradeBox>(*screen, binance_config, trade_queue);

  return App(std::move(screen), std::move(book_box), std::move(log_box),
             std::move(trade_box));
}

// main thread
void App::start() {
  // start worker threads
  book_box_->start();
  log_box_->start();
  trade_box_->start();

  // start the main UI loop,
  // Arrange in 2×2 grid via containers
  const ftxui::Component row1 =
      Horizontal({Vertical({book_box_->get_component() | flex}) | flex,
                  Vertical({trade_box_->get_component() | flex}) | flex});
  const ftxui::Component row2 =
      Horizontal({Vertical({wallet_box_.get_component() | flex}) | flex,
                  Vertical({log_box_->get_component() | flex}) | flex});
  const ftxui::Component root = Vertical({row1 | size(HEIGHT, EQUAL, 20), row2 | flex});
  screen_->loop(root);
}

}  // namespace ui
