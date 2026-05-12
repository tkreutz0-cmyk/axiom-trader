// Trade.cpp
#include "Axiom/Trade.hpp"
#include <algorithm>   // std::clamp
#include <cstdint>     // std::uint32_t

namespace Axiom {

double Trade::priceDelta() const noexcept {
    // LONG-Sicht: exit - entry
    // (Wenn SHORT wieder rein soll: TradeSide ergänzen und hier umschalten.)
    return exit - entry;
}


double Trade::calculatePips() const noexcept {
    const double delta = priceDelta();
    if (!meta.isFX) return delta; // fallback für Non-FX
    const double factor = meta.isJPY ? 100.0 : 10000.0;
    return delta * factor;
}

double Trade::calculatePnL(double pipValuePerLotUsd, bool treatAsUnits) const noexcept {
    const double delta = priceDelta();

    if (meta.isFX) {
        // FX: Pips * PipValue/Lot * Lots(=units)
        return calculatePips() * pipValuePerLotUsd * units;
    }

    // Non-FX: Delta * Units (wenn Policy aktiv)
    return treatAsUnits ? (delta * units) : delta;
}

std::uint32_t Trade::stableColorFromSymbol(std::string_view s) noexcept {
    // FNV-1a 32-bit: deterministisch über Plattformen/Läufe
    std::uint32_t h = 2166136261u;
    for (unsigned char c : s) { h ^= c; h *= 16777619u; }

    float r = ((h >> 16) & 0xFF) / 255.0f;
    float g = ((h >>  8) & 0xFF) / 255.0f;
    float b = ((h      ) & 0xFF) / 255.0f;

    auto brighten = [](float x) {
        x = x * 0.7f + 0.3f;
        return std::clamp(x, 0.0f, 1.0f);
    };
    r = brighten(r); g = brighten(g); b = brighten(b);

    const std::uint32_t R = (std::uint32_t)(r * 255.0f + 0.5f);
    const std::uint32_t G = (std::uint32_t)(g * 255.0f + 0.5f);
    const std::uint32_t B = (std::uint32_t)(b * 255.0f + 0.5f);
    const std::uint32_t A = 0xFFu;

    // Internes Format: RGBA 8:8:8:8
    return (R << 24) | (G << 16) | (B << 8) | A;
}

void Trade::refreshMetadata() noexcept {
    // Heuristik bewusst entfernt.
    // isFX / isJPY werden von außen via AssetSpec gesetzt.
    meta.assetColorRGBA = stableColorFromSymbol(symbol);
}

void Trade::setSymbol(std::string_view s) {
    symbol.assign(s.data(), s.size());
    refreshMetadata(); // nur Farbe, KEINE Logik
}



} // namespace Axiom
