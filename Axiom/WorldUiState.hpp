// WorldUiState.hpp
#pragma once
#include <string>
#include "imgui.h"

namespace Axiom {

struct WorldUiState {
    std::string activeClusterSymbol;  // leer => kein Popup
    ImVec2 activeAnchor{0.f, 0.f};

    void resetPopup() noexcept {
        activeClusterSymbol.clear();
    }

    bool hasActivePopup() const noexcept {
        return !activeClusterSymbol.empty();
    }
};

} // namespace Axiom
