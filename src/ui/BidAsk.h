#ifndef UIBIDASK_H
#define UIBIDASK_H

#include <cmath>

namespace UI {

/// @brief Binance config parameters, fetched from env
struct BidAsk{
public:
    double bid_sz = NAN;
    double bid_px = NAN;
    double ask_px = NAN;
    double ask_sz = NAN;
};

}

#endif  // UIBIDASK_H
