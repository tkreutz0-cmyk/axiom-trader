#pragma once
#include "Axiom/Trade.hpp"

namespace Axiom {

enum class PnlMode {
    PriceDelta,
    Pips,
    Money
};

struct PnlParams {
    double pipValuePerLotUsd = 10.0;
    bool treatNonFxAsUnits = true;
};

// Zentrale Umschaltlogik – KEINE Berechnung hier neu erfinden
inline double calculatePnLByMode(const Trade& t,
                                 PnlMode mode,
                                 const PnlParams& p) noexcept
{
    switch (mode) {
        case PnlMode::PriceDelta:
            return t.priceDelta();

        case PnlMode::Pips:
            return t.calculatePips();

        case PnlMode::Money:
            return t.calculatePnL(p.pipValuePerLotUsd, p.treatNonFxAsUnits);
    }
    return 0.0; // defensive default
}

} // namespace Axiom
