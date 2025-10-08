#include "app.h"

#include <quickfix/fix44/Message.h>

#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

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

App::App(moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& order_queue,
         moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& trade_queue,
         std::unique_ptr<IScreen> screen,
         std::unique_ptr<OrderBookBox> book,
         std::unique_ptr<LogBox> logs)
    : screen_(std::move(screen)),
      book_box_(std::move(book)),
      log_box_(std::move(logs)) {};

// static function
App App::from_env(
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& order_queue,
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& trade_queue) {
  std::unique_ptr<IScreen> screen = std::make_unique<FtxuiScreen>();
  auto book = std::make_unique<OrderBookBox>(*screen, order_queue);
  auto log_box = LogBox::from_env(*screen);
  return App(order_queue, trade_queue, std::move(screen), std::move(book),
             std::move(log_box));
}

// main thread
void App::start() {
  // TODO(mils): why does the order of these two items affect the rendered
  // result?

  // start worker threads
  book_box_->start();
  log_box_->start();
  // trades_.start();

  // start the main UI loop,
  // Arrange in 2×2 grid via containers
  auto row1 =
      Horizontal({Vertical({book_box_->get_component() | flex}) | size(WIDTH, EQUAL, 70),
                  Vertical({trade_box_.get_component() | flex}) | flex});
  auto row2 =
      Horizontal({Vertical({wallet_box_.get_component() | flex}) | size(WIDTH, EQUAL, 50),
                  Vertical({log_box_->get_component() | flex}) | flex});
  auto root = Vertical({row1 | size(HEIGHT, EQUAL, 10), row2 | flex});
  screen_->loop(root);
}

}  // namespace ui
