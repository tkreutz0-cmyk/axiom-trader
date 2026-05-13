// StorageCommands.hpp
#pragma once
#include <variant>
#include <future>
#include <vector>
#include <string>
#include "Axiom/DbModels.hpp"   // TradeRow, AssetSpecRow (bei dir neu) [3](https://onedrive.live.com/?id=3d8e3b0f-4ebe-45f8-bcdc-a3269016c849&cid=fb6cbfee4f7370c2&web=1)[4](https://onedrive.live.com/?id=063d3648-057d-460a-a345-093c1b3a3100&cid=fb6cbfee4f7370c2&web=1)

namespace axiom::db {

struct CmdEnsureSchema { };

struct CmdUpsertAssetSpec {
    AssetSpecRow row;
};

struct CmdInsertTrade {
    TradeRow row;
    std::promise<int64_t> outId; // optional: falls du die DB-ID zurück willst
};

struct CmdUpdateTrade {
    TradeRow row;
};

struct CmdDeleteTrade {
    int64_t id;
};

struct CmdLoadAllTrades {
    std::promise<std::vector<TradeRow>> out;
};

struct CmdShutdown { };

using DbCommand = std::variant<
    CmdEnsureSchema,
    CmdUpsertAssetSpec,
    CmdInsertTrade,
    CmdUpdateTrade,
    CmdDeleteTrade,
    CmdLoadAllTrades,
    CmdShutdown
>;

} // namespace axiom::db
