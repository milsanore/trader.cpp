#include "ui/trade_box.h"

#include <gtest/gtest.h>
#include <quickfix/fix44/MarketDataIncrementalRefresh.h>

#include <chrono>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <thread>

#include "binance/config.h"
#include "binance/side.h"
#include "binance/symbol.h"
#include "ui/app/iscreen.h"
#include "utils/double.h"
#include "utils/testing.h"

class DummyScreen : public ui::IScreen {
 public:
  void post_event(const ftxui::Event&) override {}
  void loop([[maybe_unused]] ftxui::Component renderer) override {}
};

class TradeBoxTest : public ::testing::Test {
 protected:
  DummyScreen screen_;
  moodycamel::ConcurrentQueue<FIX44::MarketDataIncrementalRefresh> queue_;
  std::unique_ptr<ui::TradeBox> trade_box_;
  binance::Config config_{"", "", "", std::vector<std::string>{}, 0, 0};

  void SetUp() override {
    trade_box_ = std::make_unique<ui::TradeBox>(screen_, config_, queue_);
    trade_box_->start();
  }
};

FIX44::MarketDataIncrementalRefresh create_valid_trade_message() {
  auto msg = FIX44::MarketDataIncrementalRefresh();
  FIX44::MarketDataIncrementalRefresh::NoMDEntries group;

  group.set(FIX::MDEntryType(FIX::MDEntryType_TRADE));
  group.set(FIX::Symbol("BTCUSDT"));
  group.set(FIX::MDEntryPx(27481.12));
  group.set(FIX::MDEntrySize(0.014));
  group.setField(FIX::TradeID("123456789"));
  group.setField(2446, "1");  // AggressorSide; 1 = Buy (Binance custom field, tag 2446)
  group.setField(60, "20231020-12:34:56.789");

  msg.set(FIX::NoMDEntries(1));
  msg.addGroup(group);
  return msg;
}

TEST_F(TradeBoxTest, RendersTradeAfterMessagePushedToQueue) {
  auto msg = create_valid_trade_message();
  queue_.enqueue(msg);

  bool trade_found = utils::Testing::wait_for(
      [&] {
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(120),
                                            ftxui::Dimension::Fixed(30));
        ftxui::Element element = trade_box_->to_table();
        ftxui::Render(screen, element);
        std::string output = screen.ToString();
        return output.find("27,481.12") != std::string::npos;
      },
      1000);

  EXPECT_TRUE(trade_found);
}
