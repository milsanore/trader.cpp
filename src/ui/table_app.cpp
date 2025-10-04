#include "table_app.h"

#include <quickfix/fix44/Message.h>

#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "concurrentqueue.h"
#include "ftxui_screen.h"
#include "iscreen.h"
#include "log_box.h"
#include "order_book_box.h"
#include "spdlog/spdlog.h"
#include "trade_box.h"
#include "wallet_box.h"

using ftxui::EQUAL;
using ftxui::flex;
using ftxui::HEIGHT;
using ftxui::size;
using ftxui::WIDTH;
using ftxui::Container::Horizontal;
using ftxui::Container::Vertical;

namespace ui {

///////////////////////////////////////

TableApp::TableApp(
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &orderQueue,
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &tradeQueue,
    std::unique_ptr<IScreen> screen, std::unique_ptr<LogBox> logs)
    : screen_(std::move(screen)), book_(*screen_, orderQueue), logs_(std::move(logs)) {};

// static function
TableApp TableApp::fromEnv(
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &orderQueue,
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &tradeQueue) {
  std::unique_ptr<IScreen> screen = std::make_unique<FtxuiScreen>();
  auto logBox = LogBox::fromEnv(*screen);
  return TableApp(orderQueue, tradeQueue, std::move(screen), std::move(logBox));
}

// main thread
void TableApp::start() {
  // TODO(mils): why does the order of these two items affect the rendered result?

  // start worker threads
  book_.Start();
  logs_->Start();
  // trades_.Start();

  // Start the main UI loop,
  // Arrange in 2×2 grid via containers
  auto row1 =
      Horizontal({Vertical({book_.GetComponent() | flex}) | size(WIDTH, EQUAL, 70),
                  Vertical({trades_.GetComponent() | flex}) | flex});
  auto row2 =
      Horizontal({Vertical({wallet_.GetComponent() | flex}) | size(WIDTH, EQUAL, 50),
                  Vertical({logs_->GetComponent() | flex}) | flex});
  auto root = Vertical({row1 | size(HEIGHT, EQUAL, 20), row2 | flex});
  screen_->Loop(root);
}

}  // namespace ui
