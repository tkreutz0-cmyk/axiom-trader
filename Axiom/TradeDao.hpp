#ifndef AXIOM_TRADE_DAO_HPP
#define AXIOM_TRADE_DAO_HPP

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include "Sqlite.hpp" // Ohne Axiom/-Präfix laden!

namespace axiom::db {

struct TradeRow;

class TradeDao {
public:
    explicit TradeDao(Connection& c);
    
    void ensureSchema();
    int64_t insert(TradeRow r);
    void update(const TradeRow& r);
    std::optional<TradeRow> getById(int64_t id);
    std::vector<TradeRow> loadAll();
    std::vector<TradeRow> loadBySymbol(const std::string& symbol);
    void removeById(int64_t id);

private:
    void prepareStatements();

    Connection& c_;
    std::optional<Statement> stInsert_;
    std::optional<Statement> stUpdate_;
    std::optional<Statement> stGetById_;
    std::optional<Statement> stLoadAll_;
    std::optional<Statement> stLoadBySymbol_;
    std::optional<Statement> stDelete_;
};

} // namespace axiom::db

#endif // AXIOM_TRADE_DAO_HPP

