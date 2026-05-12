#pragma once
#include <sqlite3.h>
#include <string>
#include <stdexcept>
#include <utility>

namespace axiom::db {

struct SqliteError : std::runtime_error {
    int code;
    explicit SqliteError(int c, const std::string& msg)
        : std::runtime_error(msg), code(c) {}
};

inline void throwOnError(int rc, sqlite3* db, const char* context) {
    if (rc == SQLITE_OK || rc == SQLITE_ROW || rc == SQLITE_DONE) return;
    const char* err = db ? sqlite3_errmsg(db) : "no db";
    throw SqliteError(rc, std::string(context) + ": " + err);
}

class Connection {
public:
    Connection() = default;
    explicit Connection(const char* path) { open(path); }
    ~Connection() { close(); }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&& other) noexcept : db_(std::exchange(other.db_, nullptr)) {}
    Connection& operator=(Connection&& other) noexcept {
        if (this != &other) {
            close();
            db_ = std::exchange(other.db_, nullptr);
        }
        return *this;
    }

    void open(const char* path) {
        close();
        int rc = sqlite3_open(path, &db_);
        if (rc != SQLITE_OK) {
            const char* err = db_ ? sqlite3_errmsg(db_) : "open failed";
            throw SqliteError(rc, std::string("sqlite3_open: ") + err);
        }
        // sinnvolle Defaults (optional):
        exec("PRAGMA foreign_keys = ON;");
        exec("PRAGMA journal_mode = WAL;");
        exec("PRAGMA synchronous = NORMAL;");
    }

    void close() {
        if (db_) sqlite3_close(db_);
        db_ = nullptr;
    }

    sqlite3* handle() const noexcept { return db_; }

    void exec(const char* sql) {
        char* errMsg = nullptr;
        int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::string msg = errMsg ? errMsg : "exec failed";
            sqlite3_free(errMsg);
            throw SqliteError(rc, msg);
        }
    }

private:
    sqlite3* db_ = nullptr;
};

class Statement {
public:
    Statement() = default;
    Statement(sqlite3* db, const char* sql) { prepare(db, sql); }
    ~Statement() { finalize(); }

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    Statement(Statement&& other) noexcept
        : db_(other.db_), stmt_(std::exchange(other.stmt_, nullptr)) {}

    Statement& operator=(Statement&& other) noexcept {
        if (this != &other) {
            finalize();
            db_ = other.db_;
            stmt_ = std::exchange(other.stmt_, nullptr);
        }
        return *this;
    }

    void prepare(sqlite3* db, const char* sql) {
        finalize();
        db_ = db;
        int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt_, nullptr);
        throwOnError(rc, db_, "sqlite3_prepare_v2");
    }

    void reset() {
        sqlite3_reset(stmt_);
        sqlite3_clear_bindings(stmt_);
    }

    int step() {
        int rc = sqlite3_step(stmt_);
        if (rc != SQLITE_ROW && rc != SQLITE_DONE) throwOnError(rc, db_, "sqlite3_step");
        return rc;
    }

    sqlite3_stmt* handle() const noexcept { return stmt_; }

    // Bind helpers
    void bindInt(int idx, int v) { throwOnError(sqlite3_bind_int(stmt_, idx, v), db_, "bind_int"); }
    void bindInt64(int idx, sqlite3_int64 v) { throwOnError(sqlite3_bind_int64(stmt_, idx, v), db_, "bind_int64"); }
    void bindDouble(int idx, double v) { throwOnError(sqlite3_bind_double(stmt_, idx, v), db_, "bind_double"); }
    void bindText(int idx, const std::string& v) {
        throwOnError(sqlite3_bind_text(stmt_, idx, v.c_str(), (int)v.size(), SQLITE_TRANSIENT), db_, "bind_text");
    }
    void bindNull(int idx) { throwOnError(sqlite3_bind_null(stmt_, idx), db_, "bind_null"); }

    // Column helpers
    int colInt(int idx) const { return sqlite3_column_int(stmt_, idx); }
    sqlite3_int64 colInt64(int idx) const { return sqlite3_column_int64(stmt_, idx); }
    double colDouble(int idx) const { return sqlite3_column_double(stmt_, idx); }
    std::string colText(int idx) const {
        const unsigned char* t = sqlite3_column_text(stmt_, idx);
        int n = sqlite3_column_bytes(stmt_, idx);
        return t ? std::string((const char*)t, (size_t)n) : std::string();
    }
    bool colIsNull(int idx) const { return sqlite3_column_type(stmt_, idx) == SQLITE_NULL; }

private:
    void finalize() {
        if (stmt_) sqlite3_finalize(stmt_);
        stmt_ = nullptr;
    }

    sqlite3* db_ = nullptr;
    sqlite3_stmt* stmt_ = nullptr;
};

class Transaction {
public:
    explicit Transaction(Connection& c) : c_(c), committed_(false) { c_.exec("BEGIN;"); }
    ~Transaction() { if (!committed_) { try { c_.exec("ROLLBACK;"); } catch (...) {} } }
    void commit() { c_.exec("COMMIT;"); committed_ = true; }
private:
    Connection& c_;
    bool committed_;
};

} // namespace axiom::db
