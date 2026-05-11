//
//  AxiomTrade.hpp
//  AxiomTrader
//
//  Created by Thorsten Kreutz on 11.05.26.
//
#include "Trade.hpp"
#include <algorithm>

namespace Axiom {

double Trade::priceDelta() const noexcept {
    // Einfache Differenz (kann negativ sein)
    return exit - entry;
}

double Trade::calculatePips() const noexcept {
    if (!meta.isFX) return priceDelta();
    double pipFactor = meta.isJPY ? 100.0 : 10000.0;
    return priceDelta() * pipFactor;
}

double Trade::calculatePnL(double pipValuePerLotUsd, bool treatAsUnits) const noexcept {
    if (meta.isFX) {
        return calculatePips() * pipValuePerLotUsd * units;
    }
    // Non-FX Fallback (Crypto/Equity)
    return treatAsUnits ? (priceDelta() * units) : priceDelta();
}

void Trade::refreshMetadata() {
    meta.isFX  = looksLikeFx(symbol);
    meta.isJPY = isJpyQuote(symbol);
    meta.assetColorRGBA = stableColorFromSymbol(symbol);
}

void Trade::setSymbol(std::string s) {
    symbol = std::move(s);
    refreshMetadata();
}

// --- Private Helpers (Statisch & Schneller) ---

bool Trade::looksLikeFx(std::string_view s) noexcept {
    auto pos = s.find('/');
    // Strengere Prüfung: "XXX/YYY" Format
    return (pos != std::string_view::npos && pos == 3 && s.size() >= 7);
}

bool Trade::isJpyQuote(std::string_view s) noexcept {
    auto pos = s.find('/');
    if (pos == std::string_view::npos) return false;
    return s.substr(pos + 1) == "JPY";
}

std::uint32_t Trade::stableColorFromSymbol(std::string_view s) noexcept {
    // Deterministischer Hash statt std::hash (für plattformübergreifende Stabilität)
    size_t hash = 5381;
    for (char c : s) hash = ((hash << 5) + hash) + c;

    auto r = static_cast<uint8_t>(((hash & 0xFF0000) >> 16) * 0.7 + 76);
    auto g = static_cast<uint8_t>(((hash & 0x00FF00) >> 8) * 0.7 + 76);
    auto b = static_cast<uint8_t>((hash & 0x0000FF) * 0.7 + 76);
    
    // RGBA Output (ImGui kompatibel)
    return (0xFFu << 24) | (b << 16) | (g << 8) | r;
}

} // namespace Axiom

