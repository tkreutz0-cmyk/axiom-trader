#pragma once

#include "Sqlite.hpp"
#include "DbModels.hpp"

#include <optional>
#include <string>
#include <vector>

namespace axiom::db {

class TradeDao {
public:
    explicit TradeDao(Connection& c);

    void ensureSchema();

    int64_t insert(TradeRow row);              // returns new id
    void update(const TradeRow& row);          // by id
    std::optional<TradeRow> getById(int64_t id);
    std::vector<TradeRow> loadAll();           // for initial hydration
    std::vector<TradeRow> loadBySymbol(const std::string& symbol);
    void removeById(int64_t id);

private:
    void prepareStatements();                  // <-- neu: lazy prepare nach ensureSchema()

    Connection& c_;

    // Lazy prepared statements (erst nach ensureSchema / prepareStatements verfügbar)
    std::optional<Statement> stInsert_;
    std::optional<Statement> stUpdate_;
    std::optional<Statement> stGetById_;
    std::optional<Statement> stLoadAll_;
    std::optional<Statement> stLoadBySymbol_;
    std::optional<Statement> stDelete_;
};

} // namespace axiom::db
