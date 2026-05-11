// PnlFormatter.hpp
#pragma once
#include <string>
#include <cstdio>
#include "Pnl.hpp"

inline std::string formatPnlLabel(const Trade& t, PnlMode mode, const PnlParams& p) {
    char out[64];
    switch (mode) {
        case PnlMode::PriceDelta:
            if (t.fx.isFx) std::snprintf(out, sizeof(out), "Δ %.5f", t.priceDelta());
            else           std::snprintf(out, sizeof(out), "Δ %.2f", t.priceDelta());
            break;
        case PnlMode::Pips:
            if (t.fx.isFx) std::snprintf(out, sizeof(out), "%.1f pips", t.pips());
            else           std::snprintf(out, sizeof(out), "%.5f", t.pips());
            break;
        case PnlMode::Money:
            std::snprintf(out, sizeof(out), "%.2f", pnlMoney(t, p));
            break;
    }
    return out;
}≈

