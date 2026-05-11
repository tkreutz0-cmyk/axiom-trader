//
//  Trade.cpp
//  AxiomTrader
//
//  Created by Thorsten Kreutz on 11.05.26.
//

#include "Trade.hpp"
#include <functional>  // std::hash
#include <algorithm>   // std::clamp

namespace Axiom {

double Trade::priceDelta() const noexcept {
    // Side ist in deiner neuen Skizze nicht mehr drin.
    // Wenn du LONG/SHORT weiterhin brauchst, nimm wieder TradeSide auf.
    // Für jetzt: delta = exit - entry (LONG-Sicht).
    return exit - entry;
}

bool Trade::looksLikeFx(std::string_view s) noexcept {
    // Heuristik wie vorher: "EUR/USD" => '/' an Pos 3, min 7 Zeichen
    const auto pos = s.find('/');
    return (pos == 3 && s.size() >= 7);
}

bool Trade::isJpyQuote(std::string_view s) noexcept {
    const auto pos = s.find('/');
    if (pos == std::string_view::npos) return false;
    const auto quote = s.substr(pos + 1);
    return quote == "JPY";
}

double Trade::calculatePips() const noexcept {
    const double delta = priceDelta();
    if (!meta.isFX) return delta; // fallback für Non-FX (deterministisch)
    const double factor = meta.isJPY ? 100.0 : 10000.0;
    return delta * factor;
}

double Trade::calculatePnL(double pipValuePerLotUsd, bool treatAsUnits) const noexcept {
    const double delta = priceDelta();

    if (meta.isFX) {
        // FX: Pips * PipValue/Lot * Lots
        const double pips = calculatePips();
        return pips * pipValuePerLotUsd * units;
    }

    // Non-FX: Delta * Units (wenn Policy aktiv)
    return treatAsUnits ? (delta * units) : delta;
}

std::uint32_t Trade::stableColorFromSymbol(std::string_view s) noexcept {
    // Stable-ish hash -> RGB; Alpha fix.
    // Hinweis: std::hash ist pro Prozess i.d.R. stabil, aber nicht garantiert zwischen Programmläufen.
    // Wenn du „über Läufe stabil“ brauchst: nimm FNV-1a 32-bit (unten).
    //
    // Variante 1 (einfach):
    // size_t h = std::hash<std::string_view>{}(s);

    // Variante 2 (über Läufe stabil): FNV-1a 32-bit
    std::uint32_t h = 2166136261u;
    for (unsigned char c : s) {
        h ^= c;
        h *= 16777619u;
    }

    // RGB aus Hash
    float r = ((h >> 16) & 0xFF) / 255.0f;
    float g = ((h >> 8)  & 0xFF) / 255.0f;
    float b = ((h)       & 0xFF) / 255.0f;

    // wie vorher: etwas aufhellen (r*0.7+0.3)
    auto brighten = float x {
        x = x * 0.7f + 0.3f;
        return std::clamp(x, 0.0f, 1.0f);
    };
    r = brighten(r); g = brighten(g); b = brighten(b);

    const std::uint32_t R = (std::uint32_t)(r * 255.0f + 0.5f);
    const std::uint32_t G = (std::uint32_t)(g * 255.0f + 0.5f);
    const std::uint32_t B = (std::uint32_t)(b * 255.0f + 0.5f);
    const std::uint32_t A = 0xFF;

    // RGBA 8:8:8:8
    return (R << 24) | (G << 16) | (B << 8) | (A);
}

void Trade::refreshMetadata() {
    meta.isFX  = looksLikeFx(symbol);
    meta.isJPY = meta.isFX ? isJpyQuote(symbol) : false;
    meta.assetColorRGBA = stableColorFromSymbol(symbol);
}

void Trade::setSymbol(std::string s) {
    symbol = std::move(s);
    refreshMetadata();
}

} // namespace Axiom
