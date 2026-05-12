#pragma once
#include "Sqlite.hpp"
#include "DbModels.hpp"
#include <optional>
#include <vector>

namespace axiom::db {

class AssetSpecDao {
public:
    explicit AssetSpecDao(Connection& c);

    void ensureSchema(); // optional: legt Tabellen an, falls du das hier machen willst

    void upsert(const AssetSpecRow& row);
    std::optional<AssetSpecRow> getBySymbol(const std::string& symbol);
    std::vector<AssetSpecRow> getAll();
    bool exists(const std::string& symbol);

private:
    Connection& c_;

    Statement stUpsert_;
    Statement stGetBySymbol_;
    Statement stGetAll_;
    Statement stExists_;
};

} // namespace axiom::db
