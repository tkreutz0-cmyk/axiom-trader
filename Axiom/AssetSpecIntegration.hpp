#pragma once
#include <string_view>
#include <chrono>
#include "Axiom/Trade.hpp"
#include "Axiom/DbModels.hpp"
#include "Axiom/AssetSpecDao.hpp"
#include "src/core/utils/fixed_point.hpp"

namespace axiom::db::asset {

// ------------------------------------------------------------
// Hilfsfunktionen
// ------------------------------------------------------------
inline int64_t nowUtcEpochSeconds() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

// Übergangs-Default (HEURISTIK NUR HIER!)
inline AssetSpecRow defaultSpecFromSymbol(std::string_view symbol) {
    AssetSpecRow r;
    r.symbol = std::string(symbol);
    
    const bool looksFx = (symbol.size() >= 7 && symbol[3] == '/');
    const bool isJpy   = (looksFx && symbol.substr(symbol.size() - 3) == "JPY");
    
    r.assetType = looksFx ? AssetType::FX : AssetType::Crypto;
    
    // ✅ PRIO 3: Erzeugung der Standardwerte als skalierte Ganzzahlen (Factor 10000) statt double
    r.pipSizeRaw      = looksFx ? (isJpy ? 100 : 1) : 10000; // 0.01 / 0.0001 / 1.0 * 10000
    r.contractSizeRaw = looksFx ? 1000000000LL : 10000;      // 100000.0 / 1.0 * 10000
    r.createdAt       = nowUtcEpochSeconds();
    return r;
}

// ------------------------------------------------------------
// AssetSpec sicherstellen (DB-Garantie)
// ------------------------------------------------------------
inline AssetSpecRow ensureAssetSpec(AssetSpecDao& dao, std::string_view symbol)
{
    if (auto existing = dao.getBySymbol(std::string(symbol))) {
        return *existing;
    }
    AssetSpecRow created = defaultSpecFromSymbol(symbol);
    dao.upsert(created);
    return created;
}

// ------------------------------------------------------------
// AssetSpec -> Trade.meta
// ------------------------------------------------------------
inline void applyToTrade(Axiom::Trade& trade, const AssetSpecRow& spec)
{
    trade.meta.isFX = (spec.assetType == AssetType::FX);
    
    // ✅ PRIO 3: Typprüfung über den Rohwert (0.1 * 10000 = 1000) anstelle von double-Vergleichen
    trade.meta.isJPY = (spec.assetType == AssetType::FX && spec.pipSizeRaw < 1000);
    
    // Farbe bleibt deterministisch über setSymbol()
    // → assetColorRGBA nicht hier überschreiben
}

} // namespace axiom::db::asset

