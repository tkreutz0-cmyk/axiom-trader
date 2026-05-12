#pragma once
#include "Sqlite.hpp"
#include "DbModels.hpp"
#include <optional>
#include <vector>

namespace axiom::db {

class AssetSpecDao {
public:
    explicit AssetSpecDao(Connection& c);

    void ensureSchema();

    void upsert(const AssetSpecRow& r);
    std::optional<AssetSpecRow> getBySymbol(const std::string& symbol);
    std::vector<AssetSpecRow> getAll();
    bool exists(const std::string& symbol);

private:
    void prepareStatements();

    Connection& c_;
    std::optional<Statement> stUpsert_;
    std::optional<Statement> stGetBySymbol_;
    std::optional<Statement> stGetAll_;
    std::optional<Statement> stExists_;
};

} // namespace axiom::db
