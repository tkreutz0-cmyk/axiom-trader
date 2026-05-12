#pragma once
#include "Sqlite.hpp"
#include "DbModels.hpp"
#include <optional>
#include <vector>

namespace axiom::db {

class TradeDao {
public:
    explicit TradeDao(Connection& c);

    void ensureSchema();

    int64_t insert(TradeRow row);         // returns new id
    void update(const TradeRow& row);     // by id
    std::optional<TradeRow> getById(int64_t id);
    std::vector<TradeRow> loadAll();      // for initial hydration
    std::vector<TradeRow> loadBySymbol(const std::string& symbol);
    void removeById(int64_t id);

private:
    Connection& c_;

    Statement stInsert_;
    Statement stUpdate_;
    Statement stGetById_;
    Statement stLoadAll_;
    Statement stLoadBySymbol_;
    Statement stDelete_;
};

} // namespace axiom::db
