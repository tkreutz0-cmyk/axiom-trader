// WorldModel.hpp
#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>      // size_t
#include "Axiom/Trade.hpp"

namespace Axiom {

struct GeoPos {
    float lon = 0.f;
    float lat = 0.f;
};

struct Cluster {
    std::string symbol;
    GeoPos pos{};
    std::vector<int> tradeIndices; // Index in trades
    int longCount  = 0;
    int shortCount = 0;
    bool hasSelected = false;
};

using LocationMap = std::unordered_map<std::string, GeoPos>;

inline std::vector<Cluster>
buildClusters(const std::vector<Trade>& trades,
              const LocationMap& locations,
              int selectedTradeId)
{
    std::unordered_map<std::string, std::size_t> indexOf;
    std::vector<Cluster> out;
    out.reserve(trades.size());

    auto getOrCreate = [&](const std::string& sym) -> Cluster& {
        auto it = indexOf.find(sym);
        if (it != indexOf.end())
            return out[it->second];

        const std::size_t idx = out.size();
        indexOf.emplace(sym, idx);

        Cluster c{};
        c.symbol = sym;

        auto loc = locations.find(sym);
        c.pos = (loc != locations.end()) ? loc->second
                                         : GeoPos{0.f, 20.f};

        out.push_back(std::move(c));
        return out.back();
    };

    for (int i = 0; i < static_cast<int>(trades.size()); ++i) {
        const Trade& t = trades[i];
        Cluster& c = getOrCreate(t.symbol);

        c.tradeIndices.push_back(i);

        // LONG / SHORT Statistik
        if (t.side == TradeSide::Long)
            ++c.longCount;
        else
            ++c.shortCount;

        if (t.id == selectedTradeId)
            c.hasSelected = true;
    }

    return out;
}

} // namespace Axiom
