#pragma once
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "Axiom/Trade.hpp"
#include "Axiom/DbModels.hpp"   // TradeRow, TradeSide, etc. (liegt bei dir im Repo)
                               // TradeRow-Felder werden von TradeDao benutzt. 

namespace axiom::db::mapping {

inline int64_t nowUtcEpochSeconds() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

// ------------------------------------------------------------
// DB -> Core (Hydration)
// ------------------------------------------------------------
// Nutzt setSymbol(), damit meta (FX/JPY/Farbe) konsistent rekonstruiert wird.
  
inline void applyRow(Axiom::Trade& t, const axiom::db::TradeRow& r) {
    t.id = static_cast<int>(r.id);

    // setSymbol() ruft refreshMetadata() auf (über Trade.cpp Implementierung) 
    t.setSymbol(std::string_view{r.symbol});

    t.entry = r.entryPrice;

    // exit_price kann NULL sein (TradeDao liest als optional). 
    // Wenn NULL: wir setzen exit = entry (später kann "open trade" explizit modelliert werden).
    t.exit  = r.exitPrice ? *r.exitPrice : r.entryPrice;

    t.units = r.quantity;
}

// ------------------------------------------------------------
// Core -> DB (Persistenz)
// ------------------------------------------------------------
// Hinweis: Der Core ist aktuell "LONG-Sicht" (priceDelta = exit-entry).
// Solange TradeSide noch nicht im Core existiert, persistieren wir standardmäßig LONG.
inline axiom::db::TradeRow toRow(const Axiom::Trade& t,
                                 int64_t nowEpoch = nowUtcEpochSeconds(),
                                 std::string venue = "",
                                 std::optional<std::string> comment = std::nullopt,
                                 std::optional<int64_t> entryTime = std::nullopt,
                                 std::optional<int64_t> exitTime  = std::nullopt,
                                 bool storeExitAsNullWhenEqualEntry = false) {
    axiom::db::TradeRow r;

    // IDs: Core nutzt int, DB nutzt int64.
