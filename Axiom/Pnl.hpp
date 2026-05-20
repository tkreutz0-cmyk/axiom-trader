#pragma once
#include "Axiom/Trade.hpp"
#include "src/core/utils/fixed_point.hpp"

namespace Axiom {

enum class PnlMode {
    PriceDelta,
    Pips,
    Money
};

struct PnlParams {
    // ✅ PRIO 2: Parameter nutzen native Money-Instanzen
    core::utils::Money pipValuePerLotUsd = core::utils::Money(10);
    bool treatNonFxAsUnits = true;
};

[[nodiscard]] inline core::utils::Money calculatePnLByMode(
    const Trade& t,
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
    return core::utils::Money::from_raw(0); // Defensiver, allokationsfreier Standardwert
}

} // namespace Axiom

