//
//  WorldUIState.hpp
//  AxiomTrader
//
//  Created by Thorsten Kreutz on 11.05.26.
//

// WorldUiState.hpp
#pragma once
#include <string>
#include "imgui.h"

struct WorldUiState {
    std::string activeClusterSymbol;
    ImVec2 activeAnchor{0,0};
    bool popupOpen = false;
};

