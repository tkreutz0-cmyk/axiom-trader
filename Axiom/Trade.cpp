#include "Trade.hpp"
#include "../src/core/utils/fixed_point.hpp"
#include <algorithm>
#include <cstdint>

namespace Axiom {

[[nodiscard]] core::utils::Money Trade::priceDelta() const noexcept {
    return exit - entry;
}

[[nodiscard]] core::utils::Money Trade::calculatePips() const noexcept {
    const core::utils::Money delta = priceDelta();
    if (!meta.isFX) return delta;
    
    const core::utils::Money factor = meta.isJPY ? core::utils::Money(100) : core::utils::Money(10000);
    return delta * factor;
}

[[nodiscard]] core::utils::Money Trade::calculatePnL(core::utils::Money pipValuePerLotUsd, bool treatAsUnits) const noexcept {
    const core::utils::Money delta = priceDelta();
    if (meta.isFX) {
        return calculatePips() * pipValuePerLotUsd * units;
    }
    return treatAsUnits ? (delta * units) : delta;
}

[[nodiscard]] std::uint32_t Trade::stableColorFromSymbol(std::string_view s) noexcept {
    std::uint32_t h = 2166136261u;
    for (unsigned char c : s) {
        h ^= c;
        h *= 16777619u;
    }
    float r = ((h >> 16) & 0xFF) / 255.0f;
    float g = ((h >> 8) & 0xFF) / 255.0f;
    float b = ((h) & 0xFF) / 255.0f;
    
    auto brighten = [](float x) {
        x = x * 0.7f + 0.3f;
        return std::clamp(x, 0.0f, 1.0f);
    };
    
    r = brighten(r); g = brighten(g); b = brighten(b);
    const std::uint32_t R = static_cast<std::uint32_t>(r * 255.0f + 0.5f);
    const std::uint32_t G = static_cast<std::uint32_t>(g * 255.0f + 0.5f);
    const std::uint32_t B = static_cast<std::uint32_t>(b * 255.0f + 0.5f);
    const std::uint32_t A = 0xFFu;
    
    return (R << 24) | (G << 16) | (B << 8) | A;
}

void Trade::refreshMetadata() noexcept {
    meta.assetColorRGBA = stableColorFromSymbol(symbol);
}

void Trade::setSymbol(std::string_view s) {
    symbol.assign(s.data(), s.size());
    refreshMetadata();
}

} // namespace Axiom

