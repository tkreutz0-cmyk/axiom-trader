///
//  Trade.hpp
//  AxiomTrader
//
//  Created by Thorsten Kreutz on 11.05.26.
//

#pragma once
#include <string>
#include <cstdint>
#include <string_view>

namespace Axiom {

    class Trade {
    public:
        int id = -1;
        std::string symbol;   // z.B. "EUR/USD" oder "BTC/USD"
        double entry = 0.0;
        double exit  = 0.0;
        double units = 1.0;   // Lots (FX) oder Units (Non-FX) – je nach Policy

        // Deterministische Logik (kein UI-Code)
        double priceDelta() const noexcept;
        double calculatePips() const noexcept;
        double calculatePnL(double pipValuePerLotUsd, bool treatAsUnits) const noexcept;

        struct Metadata {
            bool isFX  = false;
            bool isJPY = false;
            std::uint32_t assetColorRGBA = 0xFFFFFFFFu; // RGBA 8:8:8:8
        } meta;

        // Wird beim Laden/Ändern des Symbols gerufen
        
        void setSymbol(std::string_view s);
        void refreshMetadata() noexcept;

        
        
        

    private:
        static std::uint32_t stableColorFromSymbol(std::string_view s) noexcept;
    };

} // namespace Axiom

