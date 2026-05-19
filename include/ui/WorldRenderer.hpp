#pragma once
#include <vector>
#include "imgui.h"
#include "Axiom/Trade.hpp"

namespace Axiom {

class WorldRenderer {
public:
    void draw(
        ImDrawList* dl,
        ImVec2 canvasPos,
        ImVec2 canvasSize,
        const std::vector<Trade>& trades,
        int& selectedId,
        ImTextureID mapTex)
    {
        // ✅ NUR Punkte zeichnen – KEIN ImGui::Begin hier!
        for (const auto& t : trades)
        {
            float u = (t.lon_deg + 180.0f) / 360.0f;
            float v = (90.0f - t.lat_deg) / 180.0f;

            ImVec2 p = ImVec2(
                canvasPos.x + u * canvasSize.x,
                canvasPos.y + v * canvasSize.y
            );

            dl->AddCircleFilled(
                p, 5.0f,
                (t.side == TradeSide::Long)
                ? IM_COL32(60,220,80,220)
                : IM_COL32(220,60,60,220)
            );
        }
    }
};

}
``
