// WorldRenderer.hpp
#pragma once
#include <vector>

#include "imgui.h"

#include "Axiom/Trade.hpp"
#include "Axiom/WorldModel.hpp"
#include "Axiom/WorldUiState.hpp"
#include "Axiom/Pnl.hpp"           // für PnlMode, PnlParams (oder PnlFormatter, wenn du es so hast)

namespace Axiom {

class WorldRenderer {
public:
    void draw(ImDrawList* dl,
              ImVec2 canvasPos, ImVec2 canvasSize,
              const std::vector<Trade>& trades,
              int& selectedId,
              ImTextureID mapTex,
              PnlMode mode,
              const PnlParams& pnlParams,
              const LocationMap& locations,
              WorldUiState& ui);
};

} // namespace Axiom
