#ifndef UITABLEAPP_H
#define UITABLEAPP_H

#include <functional>
#include <map>
#include <mutex>
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
    /// @brief start main UI loop, start FIX worker thread
    void start();
    /// if any exceptions occurred
	std::exception_ptr thread_exception;

private:
    // mutex for reading/writing to bid/ask maps
    // UI-bound, so performance is acceptable
    mutable std::mutex mutex_;

    // main thread
    ftxui::ScreenInteractive screen_ = ftxui::ScreenInteractive::TerminalOutput();

    // worker thread
    std::jthread worker_;
    // queue of messages from FIX thread
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue_;
    /// @brief poll queue for any new FIX messages, update bid/ask maps
    /// @param stoken
    void pollQueue(std::stop_token stoken);
    void OnSnapshot(const FIX44::MarketDataSnapshotFullRefresh& msg);
    void OnIncrement(const FIX44::MarketDataIncrementalRefresh& msg);
    /// @brief sorted list of bids (descending), key=price, value=size
    std::map<double, double, std::greater<double>> bidMap_;
    /// @brief sorted list of offers (ascending), key=price, value=size
    std::map<double, double> askMap_;
};

}

#endif // UITABLEAPP_H
