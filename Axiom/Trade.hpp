#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include "../src/core/utils/fixed_point.hpp"

namespace Axiom {

enum class TradeSide : std::int8_t {
    Short = -1,
    Long = 1
};

class Trade {
public:
    int id = -1;
    std::string symbol;
    core::utils::Money entry;
    core::utils::Money exit;
    core::utils::Money units;
    float lat_deg = 0.0f;
    float lon_deg = 0.0f;
    
    TradeSide side = TradeSide::Long;

    struct Meta {
        bool isFX = false;
        bool isJPY = false;
        std::uint32_t assetColorRGBA = 0;
    } meta;

    [[nodiscard]] core::utils::Money priceDelta() const noexcept;
    [[nodiscard]] core::utils::Money calculatePips() const noexcept;
    [[nodiscard]] core::utils::Money calculatePnL(core::utils::Money pipValuePerLotUsd, bool treatAsUnits) const noexcept;
    
    [[nodiscard]] static std::uint32_t stableColorFromSymbol(std::string_view s) noexcept;
    void refreshMetadata() noexcept;
    void setSymbol(std::string_view s);
};

} // namespace Axiom

