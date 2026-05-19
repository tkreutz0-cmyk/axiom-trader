// Axiom/Trade.hpp
#pragma once

#include <string>
#include <cstdint>
#include <string_view>

namespace Axiom {

// ✅ Fix 1: Das Enum MUSS vor der Klasse deklariert werden, damit die Klasse es kennt!
enum class TradeSide : std::int8_t {
    Short = -1,
    Long = 1
};

class Trade {
public:
    int id = -1;
    std::string symbol; // z.B. "EUR/USD"
    double entry = 0.0;
    double exit = 0.0;
    double units = 1.0;

    // ✅ Fix 2: Das Core-Gegenstück zur Richtungsauswertung (Ersetzt das UI-is_buy)
    TradeSide side = TradeSide::Long;

    // ✅ Fix 3: Die Geografischen Koordinaten für die Weltkarte (lon vor lat!)
    float lat_deg = 0.0f;
    float lon_deg = 0.0f;

    // Deterministische Kern-Logik (unverändert)
    double priceDelta() const noexcept;
    double calculatePips() const noexcept;
    double calculatePnL(double pipValuePerLotUsd, bool treatAsUnits) const noexcept;

    struct Metadata {
        bool isFX = false;
        bool isJPY = false;
        std::uint32_t assetColorRGBA = 0xFFFFFFFFu;
    } meta;

    void setSymbol(std::string_view s);
    void refreshMetadata() noexcept;

private:
    static std::uint32_t stableColorFromSymbol(std::string_view s) noexcept;
};

} // namespace Axiom

