//
//  PnL.hpp
//  AxiomTrader
//
//  Created by Thorsten Kreutz on 11.05.26.
//
// Pnl.hpp
#pragma once
#include "Trade.hpp"

enum class PnlMode { PriceDelta, Pips, Money };

struct PnlParams {
    double lotsOrUnits = 1.0;
    double pipValuePerLotUsd = 10.0;
    bool treatNonFxAsUnits = true;
};

inline double pnlMoney(const Trade& t, const PnlParams& p) {
    if (t.fx.isFx) return t.pips() * p.pipValuePerLotUsd * p.lotsOrUnits;
    const double d = t.priceDelta();
    return p.treatNonFxAsUnits ? d * p.lotsOrUnits : d;
}
