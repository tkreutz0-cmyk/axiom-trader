#pragma once
// src/core/StorageManager.hpp
//
// AXIOM-Trader (v0.4.0-alpha) – SYNAPSE CRUD aligned storage core
// Deterministic, UI-agnostic, RAII-safe SQLite layer.
//
// Notes:
// - Intentionally synchronous. Use from your DB-worker thread (per architecture review). [1](https://onedrive.live.com/?id=a0c451e0-ce5c-4161-beb3-185cbd4c4c20&cid=fb6cbfee4f7370c2&web=1)

#include <array>
#include <cstdint>
#include <cstdio>      // std::snprintf
#include <filesystem>
#include <limits>
#include <memory>      // std::unique_ptr
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <sqlite3.h>

namespace axiom::core {

enum class TradeDirection : std::int32_t {
    Long  = 0,
    Short = 1
};

struct Trade {
    std::uint64_t   id           = 0;   // INTEGER PRIMARY KEY (bind NULL to autogenerate)
    std::int64_t    timestamp    = 0;   // UNIX epoch (seconds/millis caller-defined) as int64
    std::string     trading_hub;        // "NYC", "LDN", "FRA", "TYO", "SYD"
    std::string     symbol;             // asset symbol
    TradeDirection  direction    = TradeDirection::Long;
    double          entry_price  = 0.0;
    double          size         = 0.0;
    double          realized_pnl = 0.0;
};

class StorageManager final {
public:
    explicit StorageManager(std::filesystem::path db_file = std::filesystem::path{"axiom_trader.db"})
        : db_file_(std::move(db_file)) {}

    ~StorageManager() = default;

    StorageManager(const StorageManager&)            = delete;
    StorageManager& operator=(const StorageManager&) = delete;

    StorageManager(StorageManager&&) noexcept            = default;
    StorageManager& operator=(StorageManager&&) noexcept = default;

    bool initializeDatabase() {
        std::scoped_lock lock(mutex_);
        last_error_.clear();

        if (!openIfNeeded_()) return false;

        if (!exec_("PRAGMA foreign_keys = ON;")) return false;

        static constexpr std::string_view kSchemaSql =
            "CREATE TABLE IF NOT EXISTS Trade ("
            "id           INTEGER PRIMARY KEY,"
            "timestamp    INTEGER NOT NULL,"
            "trading_hub  TEXT    NOT NULL CHECK(trading_hub IN ('NYC','LDN','FRA','TYO','SYD')),"
            "symbol       TEXT    NOT NULL CHECK(length(symbol) > 0),"
            "direction    INTEGER NOT NULL CHECK(direction IN (0,1)),"
            "entry_price  REAL    NOT NULL,"
            "size         REAL    NOT NULL,"
            "realized_pnl REAL    NOT NULL"
            ");";

        if (!exec_(kSchemaSql)) return false;

        if (!exec_("CREATE INDEX IF NOT EXISTS idx_trade_hub_ts ON Trade(trading_hub, timestamp, id);")) return false;
        if (!exec_("CREATE INDEX IF NOT EXISTS idx_trade_symbol ON Trade(symbol);")) return false;

        return verifyTradeSchema_();
    }

    bool saveTrade(const Trade& trade) {
        std::scoped_lock lock(mutex_);
        last_error_.clear();

        if (!openIfNeeded_()) return false;
        if (!verifyInputs_(trade)) return false;

        static constexpr std::string_view kInsertSql =
            "INSERT OR REPLACE INTO Trade "
            "(id, timestamp, trading_hub, symbol, direction, entry_price, size, realized_pnl) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

        Statement stmt;
        if (!prepare_(kInsertSql, stmt)) return false;

        int idx = 1;

        if (trade.id == 0) {
            if (!bindNull_(stmt.get(), idx++)) return false;
        } else {
            if (trade.id > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                setError_("Trade.id exceeds SQLite INTEGER range.");
                return false;
            }
            if (!bindInt64_(stmt.get(), idx++, static_cast<std::int64_t>(trade.id))) return false;
        }

        if (!bindInt64_(stmt.get(), idx++, trade.timestamp)) return false;
        if (!bindText_(stmt.get(), idx++, trade.trading_hub)) return false;
        if (!bindText_(stmt.get(), idx++, trade.symbol)) return false;
        if (!bindInt64_(stmt.get(), idx++, static_cast<std::int64_t>(directionToDb_(trade.direction)))) return false;
        if (!bindDouble_(stmt.get(), idx++, trade.entry_price)) return false;
        if (!bindDouble_(stmt.get(), idx++, trade.size)) return false;
        if (!bindDouble_(stmt.get(), idx++, trade.realized_pnl)) return false;

        const int rc = sqlite3_step(stmt.get());
        if (rc != SQLITE_DONE) {
            setSqliteError_("sqlite3_step(INSERT/REPLACE) failed", rc);
            return false;
        }
        return true;
    }

    std::vector<Trade> getAllTradesByHub(const std::string& hub) {
        std::scoped_lock lock(mutex_);
        last_error_.clear();

        std::vector<Trade> out;
        if (!openIfNeeded_()) return out;

        if (!isValidHub_(hub)) {
            setError_("Invalid trading_hub. Allowed: NYC, LDN, FRA, TYO, SYD.");
            return out;
        }

        static constexpr std::string_view kSelectSql =
            "SELECT id, timestamp, trading_hub, symbol, direction, entry_price, size, realized_pnl "
            "FROM Trade WHERE trading_hub = ? ORDER BY timestamp ASC, id ASC;";

        Statement stmt;
        if (!prepare_(kSelectSql, stmt)) return out;
        if (!bindText_(stmt.get(), 1, hub)) return out;

        while (true) {
            const int rc = sqlite3_step(stmt.get());
            if (rc == SQLITE_ROW) {
                Trade t{};
                const auto id64 = sqlite3_column_int64(stmt.get(), 0);
                t.id = (id64 < 0) ? 0ULL : static_cast<std::uint64_t>(id64);
                t.timestamp   = sqlite3_column_int64(stmt.get(), 1);
                t.trading_hub = columnText_(stmt.get(), 2);
                t.symbol      = columnText_(stmt.get(), 3);

                const int dir = sqlite3_column_int(stmt.get(), 4);
                t.direction   = dbToDirection_(dir);

                t.entry_price  = sqlite3_column_double(stmt.get(), 5);
                t.size         = sqlite3_column_double(stmt.get(), 6);
                t.realized_pnl = sqlite3_column_double(stmt.get(), 7);

                out.push_back(std::move(t));
            } else if (rc == SQLITE_DONE) {
                break;
            } else {
                setSqliteError_("sqlite3_step(SELECT) failed", rc);
                out.clear();
                break;
            }
        }
        return out;
    }

    [[nodiscard]] std::string lastError() const {
        std::scoped_lock lock(mutex_);
        return last_error_;
    }

private:
    struct SqliteDeleter {
        void operator()(sqlite3* db) const noexcept { if (db) sqlite3_close_v2(db); }
        void operator()(sqlite3_stmt* stmt) const noexcept { if (stmt) sqlite3_finalize(stmt); }
    };

    using DatabaseHandle = std::unique_ptr<sqlite3, SqliteDeleter>;
    using Statement      = std::unique_ptr<sqlite3_stmt, SqliteDeleter>;

    std::filesystem::path db_file_;
    DatabaseHandle        db_handle_{};
    mutable std::mutex    mutex_{};
    std::string           last_error_{};

private:
    bool openIfNeeded_() {
        if (db_handle_) return true;

        std::error_code ec;
        if (db_file_.has_parent_path()) {
            std::filesystem::create_directories(db_file_.parent_path(), ec);
        }

        sqlite3* db = nullptr;
        const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;

        // UTF-8 path for deterministic cross-platform behavior
        const std::string path = db_file_.u8string();

        const int rc = sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
        db_handle_.reset(db);

        if (rc != SQLITE_OK || !db_handle_) {
            const char* errmsg = db ? sqlite3_errmsg(db) : "Failed to allocate sqlite3 context.";
            std::array<char, 256> buf{};
            std::snprintf(buf.data(), buf.size(),
                          "Open database failed (Code: %d, Errmsg: %.150s)", rc, errmsg ? errmsg : "null");
            last_error_ = buf.data();
            db_handle_.reset(nullptr); // close bad handle / avoid carrying invalid state
            return false;
        }

        // Stabilize transient lock behavior (useful once DB-worker exists)
        sqlite3_busy_timeout(db_handle_.get(), 2000);
        return true;
    }

    bool exec_(std::string_view sql) {
        if (sql.empty()) {
            setError_("exec_: empty SQL string.");
            return false;
        }
        std::string safe_sql(sql); // NUL-terminated
        char* err_msg = nullptr;

        const int rc = sqlite3_exec(db_handle_.get(), safe_sql.c_str(), nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::array<char, 256> buf{};
            std::snprintf(buf.data(), buf.size(),
                          "sqlite3_exec failed (Code: %d, Details: %.150s)",
                          rc, err_msg ? err_msg : "Unknown context");
            last_error_ = buf.data();
            if (err_msg) sqlite3_free(err_msg);
            return false;
        }
        return true;
    }

    bool prepare_(std::string_view sql, Statement& out_stmt) {
        if (sql.empty()) {
            setError_("prepare_: empty SQL string.");
            return false;
        }
        std::string safe_sql(sql); // NUL-terminated for -1 length
        sqlite3_stmt* stmt = nullptr;

        const int rc = sqlite3_prepare_v2(db_handle_.get(), safe_sql.c_str(), -1, &stmt, nullptr);
        out_stmt.reset(stmt);

        if (rc != SQLITE_OK || !out_stmt) {
            setSqliteError_("sqlite3_prepare_v2 failed", rc);
            return false;
        }
        return true;
    }

    bool bindNull_(sqlite3_stmt* stmt, int idx) {
        return checkRc_(sqlite3_bind_null(stmt, idx));
    }

    bool bindInt64_(sqlite3_stmt* stmt, int idx, std::int64_t val) {
        return checkRc_(sqlite3_bind_int64(stmt, idx, static_cast<sqlite3_int64>(val)));
    }

    bool bindDouble_(sqlite3_stmt* stmt, int idx, double val) {
        return checkRc_(sqlite3_bind_double(stmt, idx, val));
    }

    bool bindText_(sqlite3_stmt* stmt, int idx, const std::string& text) {
        if (text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
            setError_("Binding failed: Text length exceeds maximum SQLite integer boundary.");
            return false;
        }
        return checkRc_(sqlite3_bind_text(stmt, idx, text.data(),
                                         static_cast<int>(text.size()), SQLITE_TRANSIENT));
    }

    static std::string columnText_(sqlite3_stmt* stmt, int col) {
        const auto* text = sqlite3_column_text(stmt, col);
        if (!text) return {};
        const int bytes = sqlite3_column_bytes(stmt, col);
        return (bytes > 0)
            ? std::string(reinterpret_cast<const char*>(text), static_cast<std::size_t>(bytes))
            : std::string{};
    }

    bool checkRc_(int rc) {
        if (rc != SQLITE_OK) {
            setSqliteError_("Data binding pipeline failed", rc);
            return false;
        }
        return true;
    }

    bool isValidHub_(std::string_view hub) const noexcept {
        return (hub == "NYC" || hub == "LDN" || hub == "FRA" || hub == "TYO" || hub == "SYD");
    }

    bool verifyInputs_(const Trade& trade) {
        if (!isValidHub_(trade.trading_hub)) {
            setError_("Validation: Specified trading hub is invalid.");
            return false;
        }
        if (trade.symbol.empty()) {
            setError_("Validation: Security asset symbol cannot be empty.");
            return false;
        }
        return true;
    }

    bool verifyTradeSchema_() {
        // Fail-fast: required columns must exist (not just table existence).
        static constexpr std::array<std::string_view, 8> required{
            "id","timestamp","trading_hub","symbol","direction","entry_price","size","realized_pnl"
        };

        Statement stmt;
        if (!prepare_("PRAGMA table_info(Trade);", stmt)) return false;

        std::vector<std::string> present;
        present.reserve(16);

        while (true) {
            const int rc = sqlite3_step(stmt.get());
            if (rc == SQLITE_ROW) {
                // PRAGMA table_info: cid, name, type, notnull, dflt_value, pk
                present.push_back(columnText_(stmt.get(), 1));
            } else if (rc == SQLITE_DONE) {
                break;
            } else {
                setSqliteError_("PRAGMA table_info(Trade) step failed", rc);
                return false;
            }
        }

        auto has = [&](std::string_view col) -> bool {
            for (const auto& p : present) if (p == col) return true;
            return false;
        };

        for (auto col : required) {
            if (!has(col)) {
                setError_(std::string("Fail-Fast Verification: Trade table missing required column '")
                          + std::string(col) + "'.");
                return false;
            }
        }
        return true;
    }

    void setError_(std::string_view msg) {
        last_error_ = std::string(msg);
    }

    void setSqliteError_(std::string_view context, int rc) {
        std::array<char, 256> buf{};
        const char* sqlite_msg = db_handle_ ? sqlite3_errmsg(db_handle_.get()) : "Disconnected environment context";

        // IMPORTANT: context is string_view -> use %.*s to avoid UB.
        std::snprintf(buf.data(), buf.size(),
                      "%.*s (Code: %d, Errmsg: %.100s)",
                      static_cast<int>(context.size()),
                      context.data(),
                      rc,
                      sqlite_msg ? sqlite_msg : "null");

        last_error_ = buf.data();
    }

    std::int32_t directionToDb_(TradeDirection dir) const noexcept {
        return static_cast<std::int32_t>(dir);
    }

    TradeDirection dbToDirection_(int val) const noexcept {
        return (val == 1) ? TradeDirection::Short : TradeDirection::Long;
    }
};

} // namespace axiom::core
