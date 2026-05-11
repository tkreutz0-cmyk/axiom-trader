// PnlFormatter.hpp
#pragma once
#include <string>
#include <cstdio>

#include "Axiom/Pnl.hpp"

namespace Axiom {

inline std::string formatPnlLabel(const Trade& t,
                                  PnlMode mode,
                                  const PnlParams& p)
{
    char out[64];

    switch (mode) {
        case PnlMode::PriceDelta:
            if (t.meta.isFX)
                std::snprintf(out, sizeof(out), "Δ %.5f", t.priceDelta());
            else
                std::snprintf(out, sizeof(out), "Δ %.2f", t.priceDelta());
            break;

        case PnlMode::Pips:
            if (t.meta.isFX)
                std::snprintf(out, sizeof(out), "%.1f pips", t.calculatePips());
            else
                std::snprintf(out, sizeof(out), "%.5f", t.calculatePips());
            break;

        case PnlMode::Money:
            std::snprintf(out, sizeof(out),
                          "%.2f",
                          t.calculatePnL(p.pipValuePerLotUsd,
                                         p.treatNonFxAsUnits));
            break;
    }

    return out;
}

} // namespace Axiom
