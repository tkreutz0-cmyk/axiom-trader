#pragma once
#include <cstdint>
#include <optional>
#include <string>

namespace axiom::db {

enum class AssetType : uint8_t {
    FX = 0,
    Commodity = 1,
    Crypto = 2,
    CFD = 3
};

enum class TradeSide : int8_t {
    Short = -1,
    Long = 1
};

struct AssetSpecRow {
    std::string symbol;
    AssetType assetType{};
    // ✅ PRIO 1: Reiner int64_t Ganzzahl-Rohwert für die Datenbank
    int64_t pipSizeRaw = 0;
    int64_t contractSizeRaw = 0;
    int64_t createdAt = 0; // unix utc
};

struct TradeRow {
    int64_t id = 0; // AUTOINCREMENT
    std::string symbol;
    TradeSide side{};
    // ✅ PRIO 1: Umstellung aller monetären Felder und Mengen auf native int64_t Rohwerte
    int64_t entryPriceRaw = 0;
    std::optional<int64_t> exitPriceRaw;
    int64_t quantityRaw = 0;
    int64_t entryTime = 0;
    std::optional<int64_t> exitTime;
    std::string venue; // "NYC", "LDN", ...
    std::optional<std::string> comment;
    int64_t createdAt = 0;
    int64_t updatedAt = 0;
};

} // namespace axiom::db

