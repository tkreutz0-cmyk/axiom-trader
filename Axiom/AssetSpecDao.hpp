#ifndef AXIOM_DB_ASSET_SPEC_DAO_HPP
#define AXIOM_DB_ASSET_SPEC_DAO_HPP

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include "Axiom/Sqlite.hpp" // ✅ Macht Statement und Connection vollständig verfügbar

namespace axiom::db {

struct AssetSpecRow;

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

#endif // AXIOM_DB_ASSET_SPEC_DAO_HPP

