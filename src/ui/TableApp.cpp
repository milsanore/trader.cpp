#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <map>
#include <ranges>
#include <string>
#include <thread>
#include <vector>
#include <quickfix/fix44/Message.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include "TableApp.h"
#include "BidAsk.h"

namespace UI {

const std::vector<BidAsk> bookToVec(std::map<double, double>& bidMap_, std::map<double, double>& askMap_){
    int rowCount = std::max(bidMap_.size(), askMap_.size());
    std::vector<BidAsk> v(rowCount);
    // bids
	int i = 0;
	for (auto&& [px, sz] : std::views::reverse(bidMap_)) {
		v[i].bid_sz = sz;
		v[i].bid_px = px;
		i++;
	}
	// offers
	i = 0;
	for (const auto& [px, sz] : askMap_) {
		v[i].ask_sz = sz;
		v[i].ask_px = px;
		i++;
	}
    return v;  
};
// Helper to pad or truncate a string to a fixed width
const std::string Pad(const std::string& input, size_t width) {
    if (input.size() >= width)
        return input.substr(0, width);
    return input + std::string(width - input.size(), ' ');
}
const std::string Pad(const double input, size_t width) {
    const std::string s = (std::isnan(input) ? "" : std::to_string(input));
    if (s.size() >= width)
        return s.substr(0, width);
    return s + std::string(width - s.size(), ' ');
}

///////////////////////////////////////

TableApp::TableApp(moodycamel::ConcurrentQueue<std::shared_ptr<FIX44::Message>> &queue) 
    : queue_(queue) {};

void TableApp::start() {
	// start worker thread
    std::jthread updater_([this](std::stop_token stoken) {
        startUpdater(stoken);
    });

    // Start the main UI loop
    auto renderer = ftxui::Renderer([&]() {
        return buildTable();
    });
    screen_.Loop(renderer);
}

ftxui::Element TableApp::buildTable() {
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
	std::vector<BidAsk> x = bookToVec(bidMap_, askMap_);
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

void TableApp::startUpdater(std::stop_token stoken) {
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
			std::shared_ptr<FIX44::Message> msg;
			while (!queue_.try_dequeue(msg)) {
				if (stoken.stop_requested())
					return;
				adaptiveBackoff();
			}

            if (auto snap = std::dynamic_pointer_cast<FIX44::MarketDataSnapshotFullRefresh>(msg)) {
                OnSnapshot(*snap);
                screen_.PostEvent(ftxui::Event::Custom); // Trigger re-render
            }
			else if (auto inc = std::dynamic_pointer_cast<FIX44::MarketDataIncrementalRefresh>(msg)) {
                OnIncrement(*inc);
                screen_.PostEvent(ftxui::Event::Custom); // Trigger re-render
			}
			else {
				std::cout << "Unknown message type" << std::endl;
			}

			// Reset backoff state
			spinCount = 0;
			sleepTimeUs = 10;
		}
		// TODO: replace with a synchronized logging library
		std::cout << "Closing worker thread..." << std::endl;
	}
	catch (...) {
		thread_exception = std::current_exception();
	}
}

void TableApp::OnSnapshot(const FIX44::MarketDataSnapshotFullRefresh& msg) {
	FIX::Symbol symbol;
	msg.get(symbol);
	// std::cout << std::format("MD snapshot message, symbol [{}]", symbol.getString()) << std::endl;
	std::string symbolValue = symbol.getValue();
	if (symbolValue != "BTCUSDT") {
		std::cout << std::format("wrong symbol, skipping snapshot. value [{}]", symbolValue) << std::endl;
		return;
	}
	bidMap_.clear();
	askMap_.clear();
	FIX::NoMDEntries noMDEntries;
	msg.get(noMDEntries);
	int numEntries = noMDEntries.getValue();
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
			std::cout << std::format("unknown bid/offer type [{}]", entryType.getString()) << std::endl;
		}
	}
}
void TableApp::OnIncrement(const FIX44::MarketDataIncrementalRefresh& msg) {
	FIX::NoMDEntries noMDEntries;
	msg.get(noMDEntries);
	int numEntries = noMDEntries.getValue();
	for (int i = 1; i <= numEntries; i++) {
		FIX44::MarketDataIncrementalRefresh::NoMDEntries group;
		msg.getGroup(i, group);

		FIX::Symbol symbol;
		if (group.isSetField(FIX::FIELD::Symbol)) {
			group.get(symbol);
		}
		std::string symbolValue = symbol.getValue();
		if (symbolValue != "BTCUSDT") {
			// std::cout << std::format("wrong symbol, skipping increment. value [{}]", symbolValue) << std::endl;
			continue;
		}

		FIX::MDUpdateAction action;
		FIX::MDEntryType entryType;
		FIX::MDEntryPx px;
		group.get(action);
		group.get(entryType);
		group.get(px);
		
		if (entryType != FIX::MDEntryType_BID && entryType != FIX::MDEntryType_OFFER) {
			std::cout << std::format("unknown entry type, skipping. value [{}]", entryType.getString()) << std::endl;
			continue;
		}

		if (action.getValue() == FIX::MDUpdateAction_NEW || action.getValue() == FIX::MDUpdateAction_CHANGE) {
			// std::cout << std::format("price upsert") << std::endl;

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
			// std::cout << std::format("price delete") << std::endl;
			if (entryType == FIX::MDEntryType_BID) {
				bidMap_.erase(px.getValue());
			} else if (entryType == FIX::MDEntryType_OFFER) {
				askMap_.erase(px.getValue());
			}
		} else {
			std::cout << std::format("unknown price action. value [{}]", action.getValue()) << std::endl;
		}
	}
}

}
