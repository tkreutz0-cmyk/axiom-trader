#pragma once
#include <string_view>
#include <chrono>

#include "Axiom/Trade.hpp"
#include "Axiom/DbModels.hpp"
#include "Axiom/AssetSpecDao.hpp"

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

    // Übergangsheuristik (später entfernbar)
    const bool looksFx =
        (symbol.size() >= 7 && symbol[3] == '/');
    const bool isJpy =
        (looksFx && symbol.substr(symbol.size() - 3) == "JPY");

    r.assetType    = looksFx ? AssetType::FX : AssetType::Crypto;
    r.pipSize      = looksFx ? (isJpy ? 0.01 : 0.0001) : 1.0;
    r.contractSize = looksFx ? 100000.0 : 1.0;
    r.createdAt    = nowUtcEpochSeconds();
    return r;
}

// ------------------------------------------------------------
// AssetSpec sicherstellen (DB-Garantie)
// ------------------------------------------------------------
inline AssetSpecRow ensureAssetSpec(
    AssetSpecDao& dao,
    std::string_view symbol)
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
inline void applyToTrade(
    Axiom::Trade& trade,
    const AssetSpecRow& spec)
{
    trade.meta.isFX  = (spec.assetType == AssetType::FX);
    trade.meta.isJPY = (spec.assetType == AssetType::FX &&
                        spec.pipSize < 0.1);

    // Farbe bleibt deterministisch über setSymbol()
    // → assetColorRGBA nicht hier überschreiben
}

} // namespace axiom::db::asset
