#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include <quickfix/fix44/Message.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include "spdlog/spdlog.h"
#include "TableApp.h"
#include "BidAsk.h"

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
/// @brief
std::vector<BidAsk> bookToVec(std::map<double, double, std::greater<double>>& bidMap, std::map<double, double>& askMap){
	const int rowCount = std::max(bidMap.size(), askMap.size());
	std::vector<BidAsk> v(rowCount);
	// bids
	int i = 0;
	for (const auto& [px, sz] : bidMap) {
		v[i].bid_sz = sz;
		v[i].bid_px = px;
		i++;
	}
	// asks
	i = 0;
	for (const auto& [px, sz] : askMap) {
		v[i].ask_sz = sz;
		v[i].ask_px = px;
		i++;
	}
	return v;
};
/// @brief generate an FTXUI table containing the order book, from the bid/ask maps
/// @return the FTXUI element that the UI will render
ftxui::Element orderbookToTable(std::map<double, double, std::greater<double>>& bidMap, std::map<double, double>& askMap) {
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
	const std::vector<BidAsk> x = bookToVec(bidMap, askMap);
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

TableApp::TableApp(moodycamel::ConcurrentQueue<std::shared_ptr<const FIX44::Message>>& queue) : queue_(queue) {};

// main thread
void TableApp::start() {
	// TODO: flip order for better readability

	// start worker thread
    std::jthread updater_([this](std::stop_token stoken) {
        pollQueue(stoken);
    });

    // Start the main UI loop
    const auto renderer = ftxui::Renderer([&]() {
        return orderbookToTable(bidMap_, askMap_);
    });
    screen_.Loop(renderer);
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

		while (!stoken.stop_requested()) {
			std::shared_ptr<const FIX44::Message> msg;
			while (!queue_.try_dequeue(msg)) {
				if (stoken.stop_requested())
					return;
				adaptiveBackoff();
			}

            if (auto snap = std::dynamic_pointer_cast<const FIX44::MarketDataSnapshotFullRefresh>(msg)) {
                OnSnapshot(*snap);
                screen_.PostEvent(ftxui::Event::Custom); // Trigger re-render
            }
			else if (auto inc = std::dynamic_pointer_cast<const FIX44::MarketDataIncrementalRefresh>(msg)) {
                OnIncrement(*inc);
                screen_.PostEvent(ftxui::Event::Custom); // Trigger re-render
			}
			else {
				spdlog::error("unknown message type");
			}

			// Reset backoff state
			spinCount = 0;
			sleepTimeUs = 10;
		}
		spdlog::info("closing worker thread...");
	}
	// TODO: log error
	catch (...) {
		spdlog::error(std::format("error in ui worker thread"));
		thread_exception = std::current_exception();
	}
}
void TableApp::OnSnapshot(const FIX44::MarketDataSnapshotFullRefresh& msg) {
	FIX::Symbol symbol;
	msg.get(symbol);
	spdlog::debug(std::format("MD snapshot message, symbol [{}]", symbol.getString()));

	if (std::string s = symbol.getValue(); s != "BTCUSDT") {
		spdlog::debug(std::format("wrong symbol, skipping snapshot. value [{}]", s));
		return;
	}

	bidMap_.clear();
	askMap_.clear();
	FIX::NoMDEntries noMDEntries;
	msg.get(noMDEntries);
	const int numEntries = noMDEntries.getValue();
	for (int i = 1; i <= numEntries; i++) {
		FIX44::MarketDataSnapshotFullRefresh::NoMDEntries group;
		msg.getGroup(i, group);
		FIX::MDEntryType entryType;
		FIX::MDEntryPx px;
		FIX::MDEntrySize sz;
		group.get(entryType);
		group.get(px);
		group.get(sz);
		if (entryType == FIX::MDEntryType_BID) {
			bidMap_[px.getValue()] = sz.getValue();
		} else if (entryType == FIX::MDEntryType_OFFER) {
			askMap_[px.getValue()] = sz.getValue();
		} else {
			spdlog::error(std::format("unknown bid/offer type [{}]", entryType.getString()));
		}
	}
}
void TableApp::OnIncrement(const FIX44::MarketDataIncrementalRefresh& msg) {
	FIX::NoMDEntries noMDEntries;
	msg.get(noMDEntries);
	const int numEntries = noMDEntries.getValue();
	std::string symbol;
	for (int i = 1; i <= numEntries; i++) {
		FIX44::MarketDataIncrementalRefresh::NoMDEntries group;
		msg.getGroup(i, group);

		// if symbol unchanged, field won't be set
		if (group.isSetField(FIX::FIELD::Symbol)) {
			FIX::Symbol smbl;
			group.get(smbl);
			if (std::string s = smbl.getValue(); ! s.empty()) {
				symbol = s;
			}
		}

		if (symbol.empty()) {
			continue;
		}
		if (symbol != "BTCUSDT") {
			continue;
		}

		FIX::MDUpdateAction action;
		FIX::MDEntryType entryType;
		FIX::MDEntryPx px;
		group.get(action);
		group.get(entryType);
		group.get(px);
		
		if (entryType != FIX::MDEntryType_BID && entryType != FIX::MDEntryType_OFFER) {
			spdlog::error(std::format("unknown entry type, skipping. value [{}]", entryType.getString()));
			continue;
		}

		if (action.getValue() == FIX::MDUpdateAction_NEW || action.getValue() == FIX::MDUpdateAction_CHANGE) {
			spdlog::debug(std::format("price upsert"));

			if (group.isSetField(FIX::FIELD::MDEntrySize)) {
				FIX::MDEntrySize sz;
				group.get(sz);
				if (entryType == FIX::MDEntryType_BID) {
					bidMap_[px.getValue()] = sz.getValue();
				} else if (entryType == FIX::MDEntryType_OFFER) {
					askMap_[px.getValue()] = sz.getValue();
				}
			}
		} else if (action.getValue() == FIX::MDUpdateAction_DELETE) {
			spdlog::debug(std::format("price delete"));

			if (entryType == FIX::MDEntryType_BID) {
				bidMap_.erase(px.getValue());
			} else if (entryType == FIX::MDEntryType_OFFER) {
				askMap_.erase(px.getValue());
			}
		} else {
			spdlog::error(std::format("unknown price action. value [{}]", action.getValue()));
		}
	}
}

}
