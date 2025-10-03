#include "TableApp.h"

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

#include "IScreen.h"
#include "LogBox.h"
#include "OrderBookBox.h"
#include "TradeBox.h"
#include "WalletBox.h"
#include "concurrentqueue.h"
#include "spdlog/spdlog.h"

namespace UI {

///////////////////////////////////////

TableApp::TableApp(
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &orderQueue,
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>> &tradeQueue,
    std::unique_ptr<IScreen> screen)
    : screen_(std::move(screen)),
      trades_(""),
      wallet_(""),
      book_(*screen_, orderQueue),
      logs_(LogBox::fromEnv(*screen_)) {};

// main thread
void TableApp::start() {
  // TODO(mils): why does the order of these two items affect the rendered result?

  // start worker threads
  book_.Start();
  logs_.Start();
  // trades_.Start();

  // Start the main UI loop,
  // Arrange in 2×2 grid via containers
  using namespace ftxui;
  auto row1 = Container::Horizontal(
      {Container::Vertical({book_.GetComponent() | flex}) | size(WIDTH, EQUAL, 70),
       Container::Vertical({trades_.GetComponent() | flex}) | flex});
  auto row2 = Container::Horizontal(
      {Container::Vertical({wallet_.GetComponent() | flex}) | size(WIDTH, EQUAL, 50),
       Container::Vertical({logs_.GetComponent() | flex}) | flex});
  auto root = Container::Vertical({row1 | size(HEIGHT, EQUAL, 20), row2 | flex});
  screen_->Loop(root);
}

}  // namespace UI
