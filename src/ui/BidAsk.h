#ifndef UIBIDASK_H
#define UIBIDASK_H

#include <cmath>

namespace UI {

/// @brief Binance config parameters, fetched from env
struct BidAsk{
public:
    BidAsk(double bidsz = NAN, double bidpx = NAN, double askpx = NAN, double asksz = NAN)
        : bid_sz(bidsz), bid_px(bidpx), ask_px(askpx), ask_sz(asksz) {}

    double bid_sz = NAN;
    double bid_px = NAN;
    double ask_px = NAN;
    double ask_sz = NAN;

    bool operator==(const BidAsk &other) const {
        return eq(bid_sz, other.bid_sz)
            && eq(bid_px, other.bid_px)
            && eq(ask_sz, other.ask_sz)
            && eq(ask_px, other.ask_px);
    }

private:
    inline bool eq (const double a, const double b) const {
        if (std::isnan(a) && std::isnan(b))
            return true;
        return a == b;
    };
};

}

#endif  // UIBIDASK_H
