#include "spdlog/spdlog.h"
#include "BidAsk.h"
#include "OrderBook.h"

namespace UI {

OrderBook::OrderBook(std::map<double, double, std::greater<>> bidMap, std::map<double, double> askMap) :
	bidMap_(std::move(bidMap)), askMap_(std::move(askMap)) {}

// move constructor
OrderBook::OrderBook(OrderBook&& other) noexcept {
	// lock other.mutex_ to ensure safe access to its internal maps while moving
	std::lock_guard lock(other.mutex_);
	bidMap_ = std::move(other.bidMap_);
	askMap_ = std::move(other.askMap_);
	// mutex_ does not move; each instance has its own mutex
}

// move-assignment constructor
OrderBook& OrderBook::operator=(OrderBook&& other) noexcept {
	if (this != &other) {
		// Lock both mutexes without deadlock
		std::scoped_lock lock(mutex_, other.mutex_);
		bidMap_ = std::move(other.bidMap_);
		askMap_ = std::move(other.askMap_);
	}
	return *this;
}
std::vector<BidAsk> OrderBook::toVector(){
	std::lock_guard lock(mutex_);
    const size_t rowCount = std::max(bidMap_.size(), askMap_.size());
	// TODO: could this be a pre-allocated/reusable vector with exactly 5000 entries?
    std::vector<BidAsk> v(rowCount);
    // bids
    int i = 0;
    for (const auto& [px, sz] : bidMap_) {
        v[i].bid_sz = sz;
        v[i].bid_px = px;
        i++;
    }
    // asks
    i = 0;
    for (const auto& [px, sz] : askMap_) {
        v[i].ask_sz = sz;
        v[i].ask_px = px;
        i++;
    }
    return v;
};

void OrderBook::applySnapshot(const FIX44::MarketDataSnapshotFullRefresh& msg) {
	std::lock_guard lock(mutex_);
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

void OrderBook::applyIncrement(const FIX44::MarketDataIncrementalRefresh& msg) {
	std::lock_guard lock(mutex_);
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
		if (symbol.empty() && symbol != "BTCUSDT") {
			spdlog::debug(std::format("wrong symbol, skipping increment. value [{}]", symbol));
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
