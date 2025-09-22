#include <algorithm>
#include <chrono>
#include <cmath>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <quickfix/fix44/Message.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include "concurrentqueue.h"
#include "spdlog/spdlog.h"
#include "TableApp.h"
#include "BidAsk.h"
#include "./../utils/Threading.h"
#include "IScreen.h"

namespace UI {

// Helper to pad or truncate a string to a fixed width
std::string Pad(const std::string& input, const size_t width) {
    if (input.size() >= width)
        return input.substr(0, width);

	std::string result = input;
	result.resize(width, ' ');
	return result;
}
std::string Pad(const double input, const size_t width) {
	return Pad(
		std::isnan(input) ? "" : std::to_string(input),
		width
	);
}
/// @brief generate an FTXUI table containing the order book, from the bid/ask maps
/// @return the FTXUI element that the UI will render
ftxui::Element orderbookToTable(OrderBook& ob) {
	ftxui::Elements table_elements;
	constexpr int num_columns = 4;
	constexpr int column_width = 15;
	const std::vector<std::string> column_headers = {"Bid Sz", "Bid", "Ask", "Ask Sz"};

	// ─────────── Header Row ───────────
	ftxui::Elements header_cells;
	for (int col = 0; col < num_columns; ++col) {
		std::string label = (col < column_headers.size())
								? column_headers[col]
								: "Col " + std::to_string(col);
		header_cells.push_back(ftxui::text(Pad(label, column_width)) | ftxui::bold);
	}

	// Combine header row and separator
	table_elements.push_back(ftxui::vbox({
		hbox(std::move(header_cells)),
		ftxui::separator()
	}));

	// ─────────── Data Rows ───────────
	const std::vector<BidAsk> x = ob.toVector();
	constexpr size_t max_rows = 15;
	const int rowCount = std::min(x.size(), max_rows);
	for (int i = 0; i < rowCount; ++i) {
		ftxui::Elements cells;
		cells.push_back(ftxui::text(Pad(x[i].bid_sz, column_width)));
		cells.push_back(ftxui::text(Pad(x[i].bid_px, column_width)));
		cells.push_back(ftxui::text(Pad(x[i].ask_px, column_width)));
		cells.push_back(ftxui::text(Pad(x[i].ask_sz, column_width)));
		table_elements.push_back(hbox(std::move(cells)));
	}
	if (rowCount >= max_rows) {
		ftxui::Elements cells;
		cells.push_back(ftxui::text(Pad("...", column_width)));
		cells.push_back(ftxui::text(Pad("...", column_width)));
		cells.push_back(ftxui::text(Pad("...", column_width)));
		cells.push_back(ftxui::text(Pad("...", column_width)));
		table_elements.push_back(hbox(std::move(cells)));
	}

	return vbox(std::move(table_elements));
};

///////////////////////////////////////

TableApp::TableApp(
	moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue,
	std::unique_ptr<OrderBook> ob,
	std::unique_ptr<IScreen> screen,
	std::function<void(std::stop_token)> task)
: queue_(queue), book_(std::move(ob)), screen_(std::move(screen)), workerTask_(std::move(task)) {

	// default behaviour
	if (! workerTask_) {
		workerTask_ = ([this](std::stop_token stoken) {
			Utils::Threading::set_thread_name("tradercppUI");
			pollQueue(stoken);
			spdlog::info("started polling queue on background thread");
		});
	}
};

// main thread
void TableApp::start() {
	// TODO: why does the order of these two items affect the rendered result?

	// start worker thread
	worker_ = std::jthread(workerTask_);

	// Start the main UI loop
    const auto renderer = ftxui::Renderer([&]() {
        return orderbookToTable(*book_);
    });
    screen_->Loop(renderer);
}

// worker thread
void TableApp::pollQueue(std::stop_token stoken) {
	try {
		/// Adaptive backoff strategy for spin+sleep polling
		/// Performs an adaptive backoff by spinning then sleeping, increasing sleep time exponentially.
		/// Yield 10x times, followed by a 2x sleep capped at 1ms
		int spinCount = 0;
		int sleepTimeUs = 10;
		auto adaptiveBackoff = [&spinCount, &sleepTimeUs]() {
			if (spinCount < 10) {
				++spinCount;
				std::this_thread::yield();
			}
			else {
				std::this_thread::sleep_for(std::chrono::microseconds(sleepTimeUs));
				sleepTimeUs = std::min(sleepTimeUs * 2, 1000);
			}
		};

		OrderBook& book = *book_;
		IScreen& screen = *screen_;
		while (!stoken.stop_requested()) {
			std::shared_ptr<const FIX44::Message> msg;
			while (!queue_.try_dequeue(msg)) {
				if (stoken.stop_requested())
					return;
				adaptiveBackoff();
			}

            if (auto snap = std::dynamic_pointer_cast<const FIX44::MarketDataSnapshotFullRefresh>(msg)) {
            	book.applySnapshot(*snap);
                screen.PostEvent(ftxui::Event::Custom); // Trigger re-render
            } else if (auto inc = std::dynamic_pointer_cast<const FIX44::MarketDataIncrementalRefresh>(msg)) {
            	book.applyIncrement(*inc);
                screen.PostEvent(ftxui::Event::Custom); // Trigger re-render
			} else {
				spdlog::error("unknown message type");
			}

			// Reset backoff state
			spinCount = 0;
			sleepTimeUs = 10;
		}
		spdlog::info("closing worker thread...");
	}
	// TODO: log error
	catch (const std::exception& e) {
		spdlog::error("error in ui worker thread, error [{}]", e.what());
		thread_exception = std::current_exception();
	} catch (...) {
		spdlog::error(std::format("error in ui worker thread, unknown error"));
		thread_exception = std::current_exception();
	}
}

}
