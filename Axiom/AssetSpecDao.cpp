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
{
    // NOTE:
    // Keine Statement-Prepares im Konstruktor!
    // Statements werden erst nach ensureSchema() vorbereitet (lazy).
}

void AssetSpecDao::prepareStatements()
{
    // Idempotent: falls bereits vorbereitet, nichts tun.
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
  pip_size REAL NOT NULL,
  contract_size REAL NOT NULL,
  created_at INTEGER NOT NULL
);
)SQL");

    // Nach dem Schema dürfen wir Statements preparen.
    prepareStatements();
}

void AssetSpecDao::upsert(const AssetSpecRow& r)
{
    // Defensive: falls jemand upsert() vor ensureSchema() aufruft,
    // werden Statements hier spätestens vorbereitet.
    prepareStatements();

    auto& st = *stUpsert_;
    st.reset();
    st.bindText(1, r.symbol);
    st.bindInt(2, static_cast<int>(r.assetType));
    st.bindDouble(3, r.pipSize);
    st.bindDouble(4, r.contractSize);
    st.bindInt64(5, r.createdAt);
    st.step(); // DONE
}

std::optional<AssetSpecRow> AssetSpecDao::getBySymbol(const std::string& symbol)
{
    prepareStatements();

    auto& st = *stGetBySymbol_;
    st.reset();
    st.bindText(1, symbol);

    if (st.step() == SQLITE_ROW) {
        AssetSpecRow r;
        r.symbol       = st.colText(0);
        r.assetType    = static_cast<AssetType>(st.colInt(1));
        r.pipSize      = st.colDouble(2);
        r.contractSize = st.colDouble(3);
        r.createdAt    = st.colInt64(4);
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
        r.symbol       = st.colText(0);
        r.assetType    = static_cast<AssetType>(st.colInt(1));
        r.pipSize      = st.colDouble(2);
        r.contractSize = st.colDouble(3);
        r.createdAt    = st.colInt64(4);
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
