#include "AssetSpecDao.hpp"
#include <cmath>

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
{
}

void AssetSpecDao::prepareStatements()
{
 if (stUpsert_.has_value()) return;
 stUpsert_.emplace(c_.handle(), SQL_ASSETSPEC_UPSERT);
 stGetBySymbol_.emplace(c_.handle(), SQL_ASSETSPEC_GET);
 stGetAll_.emplace(c_.handle(), SQL_ASSETSPEC_ALL);
 stExists_.emplace(c_.handle(), SQL_ASSETSPEC_EXISTS);
}

void AssetSpecDao::ensureSchema()
{
 c_.exec(R"SQL(
CREATE TABLE IF NOT EXISTS AssetSpec (
 symbol TEXT PRIMARY KEY,
 asset_type INTEGER NOT NULL,
 pip_size INTEGER NOT NULL,
 contract_size INTEGER NOT NULL,
 created_at INTEGER NOT NULL
);
)SQL");
 prepareStatements();
}

void AssetSpecDao::upsert(const AssetSpecRow& r)
{
 prepareStatements();
 auto& st = *stUpsert_;
 st.reset();
 st.bindText(1, r.symbol);
 st.bindInt(2, static_cast<int>(r.assetType));
 
 // Skalierung auf INTEGER (value * 10000)
 st.bindInt64(3, static_cast<int64_t>(std::round(r.pipSize * 10000.0)));
 st.bindInt64(4, static_cast<int64_t>(std::round(r.contractSize * 10000.0)));
 
 st.bindInt64(5, r.createdAt);
 st.step();
}

std::optional<AssetSpecRow> AssetSpecDao::getBySymbol(const std::string& symbol)
{
 prepareStatements();
 auto& st = *stGetBySymbol_;
 st.reset();
 st.bindText(1, symbol);
 if (st.step() == SQLITE_ROW) {
 AssetSpecRow r;
 r.symbol = st.colText(0);
 r.assetType = static_cast<AssetType>(st.colInt(1));
 
 // Zurückskalierung auf double (stored / 10000.0)
 r.pipSize = st.colInt64(2) / 10000.0;
 r.contractSize = st.colInt64(3) / 10000.0;
 
 r.createdAt = st.colInt64(4);
 return r;
 }
 return std::nullopt;
}

std::vector<AssetSpecRow> AssetSpecDao::getAll()
{
 prepareStatements();
 std::vector<AssetSpecRow> out;
 auto& st = *stGetAll_;
 st.reset();
 while (st.step() == SQLITE_ROW) {
 AssetSpecRow r;
 r.symbol = st.colText(0);
 r.assetType = static_cast<AssetType>(st.colInt(1));
 
 // Zurückskalierung auf double
 r.pipSize = st.colInt64(2) / 10000.0;
 r.contractSize = st.colInt64(3) / 10000.0;
 
 r.createdAt = st.colInt64(4);
 out.push_back(std::move(r));
 }
 return out;
}

bool AssetSpecDao::exists(const std::string& symbol)
{
 prepareStatements();
 auto& st = *stExists_;
 st.reset();
 st.bindText(1, symbol);
 return st.step() == SQLITE_ROW;
}

} // namespace axiom::db

