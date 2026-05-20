// -----------------------------------------------------------
// DB -> Core (Hydration) + Hub Mapping
// -----------------------------------------------------------
#pragma once
#include "Axiom/Trade.hpp"
#include "Axiom/DbModels.hpp"
#include "src/core/utils/fixed_point.hpp"

inline void applyRow(Axiom::Trade& t, const axiom::db::TradeRow& r)
{
    t.id = static_cast<int>(r.id);
    
    // Symbol + deterministische Metadaten
    t.setSymbol(std::string_view{r.symbol});
    
    // ✅ PRIO 3: Mapping über from_raw() ohne jegliche double-Divisionen
    t.entry = core::utils::Money::from_raw(r.entryPriceRaw);
    t.exit  = r.exitPriceRaw ? core::utils::Money::from_raw(*r.exitPriceRaw) : core::utils::Money::from_raw(r.entryPriceRaw);
    t.units = core::utils::Money::from_raw(r.quantityRaw);
    
    // 🔥 Venue → Geo Mapping (Zero-Allocation, deterministisch)
    // Keine Maps / keine Strings → nur Vergleich
    float lat = 0.0f;
    float lon = 0.0f;
    const std::string& v = r.venue;
    if (v == "NYC") {
        lat = 40.7128f; lon = -74.0060f;
    } else if (v == "LDN") {
        lat = 51.5074f; lon = -0.1278f;
    } else if (v == "FRA") {
        lat = 50.1107f; lon = 8.6821f;
    } else if (v == "TYO") {
        lat = 35.6762f; lon = 139.6503f;
    } else if (v == "SYD") {
        lat = -33.8688f; lon = 151.2093f;
    }
    
    // 🔥 direkte Übergabe an Core
    t.lat_deg = lat;
    t.lon_deg = lon;
}

