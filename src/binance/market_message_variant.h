#pragma once

#include <quickfix/fix44/MarketDataIncrementalRefresh.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>

#include <variant>

namespace binance {

/// @brief a union type for price-update messages,
/// so that they can be placed on the same queue,
/// so that order can be maintained
using MarketMessageVariant = std::variant<FIX44::MarketDataSnapshotFullRefresh,
                                          FIX44::MarketDataIncrementalRefresh>;

}  // namespace binance
