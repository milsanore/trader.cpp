#ifndef UITABLEAPP_H
#define UITABLEAPP_H

#include <thread>
#include <quickfix/fix44/Message.h>
#include "concurrentqueue.h"
#include "OrderBook.h"
#include "IScreen.h"
#include "FtxuiScreen.h"

namespace UI {

/// worker thread, consuming market updates, for the ui
class TableApp {
public:
    explicit TableApp(
        moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue,
        std::unique_ptr<OrderBook> ob = std::make_unique<OrderBook>(),
        std::unique_ptr<IScreen> screen = std::make_unique<FtxuiScreen>(),
        std::function<void(std::stop_token)> task = {}
    );
    /// @brief start main UI loop, start FIX worker thread
    void start();
    /// if any exceptions occurred
	std::exception_ptr thread_exception;

private:
    std::unique_ptr<OrderBook> book_;

    // main thread
    // ftxui::ScreenInteractive screen_ = ftxui::ScreenInteractive::TerminalOutput();
    std::unique_ptr<IScreen> screen_;

    // worker thread
    std::jthread worker_;
    std::function<void(std::stop_token)> workerTask_;
    // queue of messages from FIX thread
    moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue_;
    /// @brief poll queue for any new FIX messages, update order book
    /// @param stoken
    void pollQueue(std::stop_token stoken);
};

}

#endif // UITABLEAPP_H
