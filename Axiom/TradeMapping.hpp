// FILE: ./Axiom/TradeMapping.hpp
#pragma once
#include "Axiom/Trade.hpp"
#include "Axiom/DbModels.hpp"
#include "src/core/utils/fixed_point.hpp"

namespace Axiom::mapping {

struct StaticGeoPos {
    float lat;
    float lon;
};

/**
 * @brief Zentraler, allokationsfreier Mapping-Layer für Handelsplätze (Venues)
 */
[[nodiscard]] inline StaticGeoPos getGeoPosForVenue(const std::string& venue) noexcept {
    if (venue == "NYC") return {40.7128f, -74.0060f};
    if (venue == "LDN") return {51.5074f, -0.1278f};
    if (venue == "FRA") return {50.1107f, 8.6821f};
    if (venue == "TYO") return {35.6762f, 139.6503f};
    if (venue == "SYD") return {-33.8688f, 151.2093f};
    return {0.0f, 20.0f}; // Definiert als Standard-Fallback laut Review
}

} // namespace Axiom::mapping

/**
 * @brief Hydriert ein Core-Trade-Objekt aus einer DB-Row
 */
inline void applyRow(Axiom::Trade& t, const axiom::db::TradeRow& r)
{
    t.id = static_cast<int>(r.id);
    
    // Symbol zuweisen & Metadaten triggern
    t.setSymbol(std::string_view{r.symbol});
    
    // Konvertierung ohne double-Divisionen über from_raw
    t.entry = core::utils::Money::from_raw(r.entryPriceRaw);
    t.exit = r.exitPriceRaw
        ? core::utils::Money::from_raw(*r.exitPriceRaw)
        : core::utils::Money::from_raw(r.entryPriceRaw);
    t.units = core::utils::Money::from_raw(r.quantityRaw);
    
    // Bezug der Geodaten über den neuen Mapping-Layer
    Axiom::mapping::StaticGeoPos geo = Axiom::mapping::getGeoPosForVenue(r.venue);
    t.lat_deg = geo.lat;
    t.lon_deg = geo.lon;
}

