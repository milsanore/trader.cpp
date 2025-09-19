#ifndef UITABLEAPP_H
#define UITABLEAPP_H

#include <map>
#include <thread>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <quickfix/fix44/Message.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include "concurrentqueue.h"

namespace UI {

/// worker thread, consuming market updates, for the ui
class TableApp {
public:
    explicit TableApp(moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue);
    void start();
    /// if any exceptions occurred
	std::exception_ptr thread_exception;

private:
    // runs on main thread
    ftxui::ScreenInteractive screen_ = ftxui::ScreenInteractive::TerminalOutput();
    // consumes from FIX thread
	moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue_;
    // worker thread
    std::jthread binanceUpdater_;
    std::map<double, double> bidMap_;
    std::map<double, double> askMap_;
    //
    void startUpdater(std::stop_token stoken);
    ftxui::Element buildTable();
    void OnSnapshot(const FIX44::MarketDataSnapshotFullRefresh& msg);
    void OnIncrement(const FIX44::MarketDataIncrementalRefresh& msg);
};

}

#endif // UITABLEAPP_H
