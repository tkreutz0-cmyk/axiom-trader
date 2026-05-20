// ✅ KORREKTUR: Expliziter, lokaler Include verhindert Präprozessor-Schleifen
#include "../Axiom/TradeDao.hpp"
#include "../Axiom/DbModels.hpp"
#include "src/core/utils/fixed_point.hpp" // Holt core::utils::Money
#include <cstdint>
#include <stdexcept>
#include <sqlite3.h>

namespace axiom::db {

static constexpr const char* SQL_TRADE_INSERT =
R"SQL(
INSERT INTO Trade(
 symbol, side, entry_price, exit_price, quantity,
 entry_time, exit_time, venue, comment, created_at, updated_at
)
VALUES( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )
)SQL";

static constexpr const char* SQL_TRADE_UPDATE =
R"SQL(
UPDATE Trade SET
 symbol=?,
 side=?,
 entry_price=?,
 exit_price=?,
 quantity=?,
 entry_time=?,
 exit_time=?,
 venue=?,
 comment=?,
 updated_at=?
WHERE id=?
)SQL";

static constexpr const char* SQL_TRADE_GETBYID =
R"SQL(
SELECT
 id, symbol, side, entry_price, exit_price, quantity,
 entry_time, exit_time, venue, comment, created_at, updated_at
FROM Trade
WHERE id = ?
)SQL";

static constexpr const char* SQL_TRADE_LOADALL =
R"SQL(
SELECT
 id, symbol, side, entry_price, exit_price, quantity,
 entry_time, exit_time, venue, comment, created_at, updated_at
FROM Trade
ORDER BY entry_time ASC, id ASC
)SQL";

static constexpr const char* SQL_TRADE_LOADBYSYMBOL =
R"SQL(
SELECT
 id, symbol, side, entry_price, exit_price, quantity,
 entry_time, exit_time, venue, comment, created_at, updated_at
FROM Trade
WHERE symbol = ?
ORDER BY entry_time ASC, id ASC
)SQL";

static constexpr const char* SQL_TRADE_DELETE =
R"SQL(
DELETE FROM Trade WHERE id = ?
)SQL";

static TradeRow readTradeRow(Statement& st) {
 TradeRow r;
 r.id = st.colInt64(0);
 r.symbol = st.colText(1);
 r.side = static_cast<TradeSide>(st.colInt(2));
 r.entryPriceRaw = st.colInt64(3);
 if (st.colIsNull(4)) {
 r.exitPriceRaw = std::nullopt;
 } else {
 r.exitPriceRaw = st.colInt64(4);
 }
 r.quantityRaw = st.colInt64(5);
 r.entryTime = st.colInt64(6);
 if (st.colIsNull(7)) r.exitTime = std::nullopt;
 else r.exitTime = st.colInt64(7);
 r.venue = st.colText(8);
 if (st.colIsNull(9)) r.comment = std::nullopt;
 else r.comment = st.colText(9);
 r.createdAt = st.colInt64(10);
 r.updatedAt = st.colInt64(11);
 return r;
}

TradeDao::TradeDao(Connection& c) : c_(c) {}

void TradeDao::prepareStatements()
{
 if (stInsert_.has_value()) return;
 stInsert_.emplace(c_.handle(), SQL_TRADE_INSERT);
 stUpdate_.emplace(c_.handle(), SQL_TRADE_UPDATE);
 stGetById_.emplace(c_.handle(), SQL_TRADE_GETBYID);
 stLoadAll_.emplace(c_.handle(), SQL_TRADE_LOADALL);
 stLoadBySymbol_.emplace(c_.handle(), SQL_TRADE_LOADBYSYMBOL);
 stDelete_.emplace(c_.handle(), SQL_TRADE_DELETE);
}

void TradeDao::ensureSchema()
{
 c_.exec(R"SQL(
CREATE TABLE IF NOT EXISTS Trade (
 id INTEGER PRIMARY KEY AUTOINCREMENT,
 symbol TEXT NOT NULL,
 side INTEGER NOT NULL,
 entry_price INTEGER NOT NULL,
 exit_price INTEGER,
 quantity INTEGER NOT NULL,
 entry_time INTEGER NOT NULL,
 exit_time INTEGER,
 venue TEXT NOT NULL,
 comment TEXT,
 created_at INTEGER NOT NULL,
 updated_at INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_trade_symbol ON Trade(symbol);
CREATE INDEX IF NOT EXISTS idx_trade_entry ON Trade(entry_time);
CREATE INDEX IF NOT EXISTS idx_trade_exit ON Trade(exit_time);
)SQL");
 prepareStatements();
}

int64_t TradeDao::insert(TradeRow r)
{
 prepareStatements();
 auto& st = *stInsert_;
 st.reset();
 st.bindText(1, r.symbol);
 st.bindInt(2, static_cast<int>(r.side));
 st.bindInt64(3, r.entryPriceRaw);
 if (r.exitPriceRaw) {
 st.bindInt64(4, *(r.exitPriceRaw));
 } else {
 st.bindNull(4);
 }
 st.bindInt64(5, r.quantityRaw);
 st.bindInt64(6, r.entryTime);
 if (r.exitTime) st.bindInt64(7, *r.exitTime);
 else st.bindNull(7);
 st.bindText(8, r.venue);
 if (r.comment) st.bindText(9, *r.comment);
 else st.bindNull(9);
 st.bindInt64(10, r.createdAt);
 st.bindInt64(11, r.updatedAt);
 st.step();
 return sqlite3_last_insert_rowid(c_.handle());
}

void TradeDao::update(const TradeRow& r)
{
 prepareStatements();
 auto& st = *stUpdate_;
 st.reset();
 st.bindText(1, r.symbol);
 st.bindInt(2, static_cast<int>(r.side));
 st.bindInt64(3, r.entryPriceRaw);
 if (r.exitPriceRaw) {
 st.bindInt64(4, *(r.exitPriceRaw));
 } else {
 st.bindNull(4);
 }
 st.bindInt64(5, r.quantityRaw);
 st.bindInt64(6, r.entryTime);
 if (r.exitTime) st.bindInt64(7, *r.exitTime);
 else st.bindNull(7);
 st.bindText(8, r.venue);
 if (r.comment) st.bindText(9, *r.comment);
 else st.bindNull(9);
 st.bindInt64(10, r.updatedAt);
 st.bindInt64(11, r.id);
 st.step();
}

std::optional<TradeRow> TradeDao::getById(int64_t id)
{
 prepareStatements();
 auto& st = *stGetById_;
 st.reset();
 st.bindInt64(1, id);
 if (st.step() == SQLITE_ROW) {
 return readTradeRow(st);
 }
 return std::nullopt;
}

std::vector<TradeRow> TradeDao::loadAll()
{
 prepareStatements();
 std::vector<TradeRow> out;
 auto& st = *stLoadAll_;
 st.reset();
 while (st.step() == SQLITE_ROW) {
 out.push_back(readTradeRow(st));
 }
 return out;
}

std::vector<TradeRow> TradeDao::loadBySymbol(const std::string& symbol)
{
 prepareStatements();
 std::vector<TradeRow> out;
 auto& st = *stLoadBySymbol_;
 st.reset();
 st.bindText(1, symbol);
 while (st.step() == SQLITE_ROW) {
 out.push_back(readTradeRow(st));
 }
 return out;
}

void TradeDao::removeById(int64_t id)
{
 prepareStatements();
 auto& st = *stDelete_;
 st.reset();
 st.bindInt64(1, id);
 st.step();
}

} // namespace axiom::db

