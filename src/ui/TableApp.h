#ifndef UITABLEAPP_H
#define UITABLEAPP_H

#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "concurrentqueue.h"
#include <quickfix/fix44/Message.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include "BidAsk.h"

namespace UI {

/// worker thread, consuming market updates, for the ui
class TableApp {
public:
    TableApp(moodycamel::ConcurrentQueue<std::shared_ptr<FIX44::Message>>& queue);
    void start();
    /// if any exceptions occurred
	std::exception_ptr thread_exception;

private:
    ftxui::ScreenInteractive screen_ = ftxui::ScreenInteractive::TerminalOutput();
    std::map<double, double> bidMap_;
    std::map<double, double> askMap_;
	moodycamel::ConcurrentQueue<std::shared_ptr<FIX44::Message>>& queue_;
    std::jthread binanceUpdater_;
    //
    void startUpdater(std::stop_token stoken);
    ftxui::Element buildTable();
    void OnSnapshot(const FIX44::MarketDataSnapshotFullRefresh& msg);
    void OnIncrement(const FIX44::MarketDataIncrementalRefresh& msg);
};

}

#endif // UITABLEAPP_H
