#include "TradeDao.hpp"

namespace axiom::db {

static constexpr const char* SQL_TRADE_INSERT =
R"SQL(
INSERT INTO Trade(
  symbol, side, entry_price, exit_price, quantity,
  entry_time, exit_time, venue, comment, created_at, updated_at
) VALUES(
  ?, ?, ?, ?, ?,
  ?, ?, ?, ?, ?, ?
)
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
    r.entryPrice = st.colDouble(3);

    if (st.colIsNull(4)) r.exitPrice = std::nullopt;
    else r.exitPrice = st.colDouble(4);

    r.quantity = st.colDouble(5);
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

TradeDao::TradeDao(Connection& c)
: c_(c)
, stInsert_(c.handle(), SQL_TRADE_INSERT)
, stUpdate_(c.handle(), SQL_TRADE_UPDATE)
, stGetById_(c.handle(), SQL_TRADE_GETBYID)
, stLoadAll_(c.handle(), SQL_TRADE_LOADALL)
, stLoadBySymbol_(c.handle(), SQL_TRADE_LOADBYSYMBOL)
, stDelete_(c.handle(), SQL_TRADE_DELETE)
{}

void TradeDao::ensureSchema() {
    c_.exec(R"SQL(
CREATE TABLE IF NOT EXISTS Trade (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  symbol TEXT NOT NULL,
  side INTEGER NOT NULL,
  entry_price REAL NOT NULL,
  exit_price REAL,
  quantity REAL NOT NULL,
  entry_time INTEGER NOT NULL,
  exit_time INTEGER,
  venue TEXT NOT NULL,
  comment TEXT,
  created_at INTEGER NOT NULL,
  updated_at INTEGER NOT NULL,
  FOREIGN KEY(symbol) REFERENCES AssetSpec(symbol)
    ON UPDATE CASCADE
    ON DELETE RESTRICT
);
CREATE INDEX IF NOT EXISTS idx_trade_symbol ON Trade(symbol);
CREATE INDEX IF NOT EXISTS idx_trade_entry  ON Trade(entry_time);
CREATE INDEX IF NOT EXISTS idx_trade_exit   ON Trade(exit_time);
)SQL");
}

int64_t TradeDao::insert(TradeRow r) {
    stInsert_.reset();
    stInsert_.bindText(1, r.symbol);
    stInsert_.bindInt(2, static_cast<int>(r.side));
    stInsert_.bindDouble(3, r.entryPrice);

    if (r.exitPrice) stInsert_.bindDouble(4, *r.exitPrice);
    else stInsert_.bindNull(4);

    stInsert_.bindDouble(5, r.quantity);
    stInsert_.bindInt64(6, r.entryTime);

    if (r.exitTime) stInsert_.bindInt64(7, *r.exitTime);
    else stInsert_.bindNull(7);

    stInsert_.bindText(8, r.venue);

    if (r.comment) stInsert_.bindText(9, *r.comment);
    else stInsert_.bindNull(9);

    stInsert_.bindInt64(10, r.createdAt);
    stInsert_.bindInt64(11, r.updatedAt);

    stInsert_.step(); // DONE
    return sqlite3_last_insert_rowid(c_.handle());
}

void TradeDao::update(const TradeRow& r) {
    stUpdate_.reset();
    stUpdate_.bindText(1, r.symbol);
    stUpdate_.bindInt(2, static_cast<int>(r.side));
    stUpdate_.bindDouble(3, r.entryPrice);

    if (r.exitPrice) stUpdate_.bindDouble(4, *r.exitPrice);
    else stUpdate_.bindNull(4);

    stUpdate_.bindDouble(5, r.quantity);
    stUpdate_.bindInt64(6, r.entryTime);

    if (r.exitTime) stUpdate_.bindInt64(7, *r.exitTime);
    else stUpdate_.bindNull(7);

    stUpdate_.bindText(8, r.venue);

    if (r.comment) stUpdate_.bindText(9, *r.comment);
    else stUpdate_.bindNull(9);

    stUpdate_.bindInt64(10, r.updatedAt);
    stUpdate_.bindInt64(11, r.id);

    stUpdate_.step(); // DONE
}

std::optional<TradeRow> TradeDao::getById(int64_t id) {
    stGetById_.reset();
    stGetById_.bindInt64(1, id);

    if (stGetById_.step() == SQLITE_ROW) {
        return readTradeRow(stGetById_);
    }
    return std::nullopt;
}

std::vector<TradeRow> TradeDao::loadAll() {
    std::vector<TradeRow> out;
    stLoadAll_.reset();
    while (stLoadAll_.step() == SQLITE_ROW) {
        out.push_back(readTradeRow(stLoadAll_));
    }
    return out;
}

std::vector<TradeRow> TradeDao::loadBySymbol(const std::string& symbol) {
    std::vector<TradeRow> out;
    stLoadBySymbol_.reset();
    stLoadBySymbol_.bindText(1, symbol);
    while (stLoadBySymbol_.step() == SQLITE_ROW) {
        out.push_back(readTradeRow(stLoadBySymbol_));
    }
    return out;
}

void TradeDao::removeById(int64_t id) {
    stDelete_.reset();
    stDelete_.bindInt64(1, id);
    stDelete_.step();
}

} // namespace axiom::db

