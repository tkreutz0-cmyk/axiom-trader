// FILE: ./include/ui/WorldRenderer.hpp
#pragma once
#include <vector>
#include "imgui.h"
#include "Axiom/Trade.hpp"

namespace Axiom::ui {

/**
 * @brief Reine Core-Mathematik für die Koordinatenprojektion.
 * Plattformununabhängig, Hotpath-safe und frei von UI-Framework-Kopplung.
 */
[[nodiscard]] inline ImVec2 projectCoordinates(float lat, float lon, ImVec2 canvasPos, ImVec2 canvasSize) noexcept {
    float u = (lon + 180.0f) / 360.0f;
    float v = (90.0f - lat) / 180.0f;
    return ImVec2(
        canvasPos.x + u * canvasSize.x,
        canvasPos.y + v * canvasSize.y
    );
}

class WorldRenderer {
public:
    /**
     * @brief Isoliertes Zeichnen der geclusterten Punkte auf der UI-Ebene.
     */
    void drawClusters(ImDrawList* dl,
                      ImVec2 canvasPos,
                      ImVec2 canvasSize,
                      const std::vector<Trade>& trades)
    {
        for (const auto& t : trades)
        {
            // Nutzung des mathematischen Projektors
            ImVec2 p = projectCoordinates(t.lat_deg, t.lon_deg, canvasPos, canvasSize);
            
            dl->AddCircleFilled(
                p,
                5.0f,
                (t.side == TradeSide::Long)
                    ? IM_COL32(60, 220, 80, 220)
                    : IM_COL32(220, 60, 60, 220)
            );
        }
    }
};

} // namespace Axiom::ui

