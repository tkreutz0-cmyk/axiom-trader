// Axiom/WorldModel.hpp
#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>
#include <utility>   // ✅ Fix 1: Garantiert die Verfügbarkeit von std::move
#include <algorithm> // ✅ Fix 2: Erlaubt std::sort für 100% Determinismus

#include "Axiom/Trade.hpp"

namespace Axiom {

struct GeoPos {
    float lon = 0.f;
    float lat = 0.f;
};

struct Cluster {
    std::string symbol;
    GeoPos pos{};
    std::vector<int> tradeIndices; // Indizes in trades
    int longCount = 0;
    int shortCount = 0;
    bool hasSelected = false;
};

using LocationMap = std::unordered_map<std::string, GeoPos>;

inline std::vector<Cluster> buildClusters(
    const std::vector<Trade>& trades,
    const LocationMap& locations,
    int selectedTradeId)
{
    // Die temporäre Map hält die Allokationen im Hotpath so gering wie möglich
    std::unordered_map<std::string, Cluster> clusterMap;
    clusterMap.reserve(locations.size());

    for (int i = 0; i < static_cast<int>(trades.size()); ++i) {
        const Trade& t = trades[i];
        auto& c = clusterMap[t.symbol];

        if (c.symbol.empty()) {
            c.symbol = t.symbol;
            auto loc = locations.find(t.symbol);
            c.pos = (loc != locations.end())
                ? loc->second
                : GeoPos{0.f, 20.f}; // lon=0.f, lat=20.f
        }

        // ✅ Fix: Nur ein push_back
        c.tradeIndices.push_back(i);

        // ✅ Fix: Echte Richtungsauswertung über das Core-Enum
        if (t.side == TradeSide::Long) {
            ++c.longCount;
        } else {
            ++c.shortCount;
        }

        // ✅ Fix: Selektionsprüfung repariert
        if (t.id == selectedTradeId) {
            c.hasSelected = true;
        }
    }

    std::vector<Cluster> out;
    out.reserve(clusterMap.size());
    for (auto& pair : clusterMap) {
        out.push_back(std::move(pair.second));
    }

    // ✅ Fix 3: 100% Determinismus wiederhergestellt
    // Da die unordered_map eine unvorhersehbare Iterationsreihenfolge hat,
    // sortieren wir den Ausgabe-Vektor strikt alphabetisch nach dem Symbol.
    std::sort(out.begin(), out.end(), [](const Cluster& a, const Cluster& b) {
        return a.symbol < b.symbol;
    });

    return out;
}

} // namespace Axiom

