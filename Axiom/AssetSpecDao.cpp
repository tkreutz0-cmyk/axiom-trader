#include "AssetSpecDao.hpp"

namespace axiom::db {

static constexpr const char* SQL_ASSETSPEC_UPSERT =
R"SQL(
INSERT INTO AssetSpec(symbol, asset_type, pip_size, contract_size, created_at)
VALUES(?, ?, ?, ?, ?)
ON CONFLICT(symbol) DO UPDATE SET
  asset_type=excluded.asset_type,
  pip_size=excluded.pip_size,
  contract_size=excluded.contract_size
)SQL";

static constexpr const char* SQL_ASSETSPEC_GET =
R"SQL(
SELECT symbol, asset_type, pip_size, contract_size, created_at
FROM AssetSpec
WHERE symbol = ?
)SQL";

static constexpr const char* SQL_ASSETSPEC_ALL =
R"SQL(
SELECT symbol, asset_type, pip_size, contract_size, created_at
FROM AssetSpec
ORDER BY symbol
)SQL";

static constexpr const char* SQL_ASSETSPEC_EXISTS =
R"SQL(
SELECT 1 FROM AssetSpec WHERE symbol = ? LIMIT 1
)SQL";

AssetSpecDao::AssetSpecDao(Connection& c)
: c_(c)
, stUpsert_(c.handle(), SQL_ASSETSPEC_UPSERT)
, stGetBySymbol_(c.handle(), SQL_ASSETSPEC_GET)
, stGetAll_(c.handle(), SQL_ASSETSPEC_ALL)
, stExists_(c.handle(), SQL_ASSETSPEC_EXISTS)
{}

void AssetSpecDao::ensureSchema() {
    c_.exec(R"SQL(
CREATE TABLE IF NOT EXISTS AssetSpec (
  symbol TEXT PRIMARY KEY,
  asset_type INTEGER NOT NULL,
  pip_size REAL NOT NULL,
  contract_size REAL NOT NULL,
  created_at INTEGER NOT NULL
);
)SQL");
}

void AssetSpecDao::upsert(const AssetSpecRow& r) {
    stUpsert_.reset();
    stUpsert_.bindText(1, r.symbol);
    stUpsert_.bindInt(2, static_cast<int>(r.assetType));
    stUpsert_.bindDouble(3, r.pipSize);
    stUpsert_.bindDouble(4, r.contractSize);
    stUpsert_.bindInt64(5, r.createdAt);

    stUpsert_.step(); // DONE
}

std::optional<AssetSpecRow> AssetSpecDao::getBySymbol(const std::string& symbol) {
    stGetBySymbol_.reset();
    stGetBySymbol_.bindText(1, symbol);

    if (stGetBySymbol_.step() == SQLITE_ROW) {
        AssetSpecRow r;
        r.symbol = stGetBySymbol_.colText(0);
        r.assetType = static_cast<AssetType>(stGetBySymbol_.colInt(1));
        r.pipSize = stGetBySymbol_.colDouble(2);
        r.contractSize = stGetBySymbol_.colDouble(3);
        r.createdAt = stGetBySymbol_.colInt64(4);
        return r;
    }
    return std::nullopt;
}

std::vector<AssetSpecRow> AssetSpecDao::getAll() {
    std::vector<AssetSpecRow> out;
    stGetAll_.reset();
    while (stGetAll_.step() == SQLITE_ROW) {
        AssetSpecRow r;
        r.symbol = stGetAll_.colText(0);
        r.assetType = static_cast<AssetType>(stGetAll_.colInt(1));
        r.pipSize = stGetAll_.colDouble(2);
        r.contractSize = stGetAll_.colDouble(3);
        r.createdAt = stGetAll_.colInt64(4);
        out.push_back(std::move(r));
    }
    return out;
}

bool AssetSpecDao::exists(const std::string& symbol) {
    stExists_.reset();
    stExists_.bindText(1, symbol);
    return stExists_.step() == SQLITE_ROW;
}

} // namespace axiom::db
