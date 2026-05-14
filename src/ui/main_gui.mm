// src/ui/main_gui.mm  (Stage D1 - SQLite sync)

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <chrono>
#include <cstring>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"

// ---- UI / MAP ----
#include "MapTexture.hpp"

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

// ---- AXIOM CORE ----
#include "Axiom/Trade.hpp"

// ---- DB LAYER ----
#include "Axiom/DbModels.hpp"
#include "Axiom/Sqlite.hpp"
#include "Axiom/AssetSpecDao.hpp"
#include "Axiom/TradeDao.hpp"

static int64_t nowUtcEpochSeconds() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

// ---- AssetSpec helpers (DB-backed) ----
static bool isFx(const axiom::db::AssetSpecRow& s) {
    return s.assetType == axiom::db::AssetType::FX;
}

// JPY leiten wir aus pip_size ab: 0.01 ist typischer JPY-Pip; 0.0001 normal
static bool isJpy(const axiom::db::AssetSpecRow& s) {
    return isFx(s) && (s.pipSize >= 0.009 && s.pipSize <= 0.011);
}

static void applySpecToTrade(Axiom::Trade& t, const axiom::db::AssetSpecRow& s) {
    t.meta.isFX  = isFx(s);
    t.meta.isJPY = isJpy(s);
}

// UI-only default for first-time symbol (wird sofort persistiert via upsert)
static axiom::db::AssetSpecRow defaultSpecForSymbol(const std::string& symbol) {
    axiom::db::AssetSpecRow r;
    r.symbol = symbol;
    r.createdAt = nowUtcEpochSeconds();

    bool looksFx = (symbol.size() >= 7 && symbol[3] == '/');
    if (looksFx) {
        r.assetType = axiom::db::AssetType::FX;
        bool quoteJpy = (symbol.size() >= 7 && symbol.substr(symbol.size() - 3) == "JPY");
        r.pipSize = quoteJpy ? 0.01 : 0.0001;
        r.contractSize = 100000.0;
    } else {
        r.assetType = axiom::db::AssetType::Crypto;
        r.pipSize = 1.0;
        r.contractSize = 1.0;
    }
    return r;
}

// ---- TradeRow mapping (minimal, passend zu deinem TradeDao insert/update) ----
// TradeDao::insert benötigt symbol, side, entry_price, exit_price, quantity, entry_time, exit_time, venue, comment, created_at, updated_at
static axiom::db::TradeRow toTradeRow(const Axiom::Trade& t, bool isInsert) {
    axiom::db::TradeRow r;
    r.id = (t.id >= 0) ? (int64_t)t.id : 0;
    r.symbol = t.symbol;

    // side ist im Core noch nicht modelliert -> Long als Übergang
    r.side = axiom::db::TradeSide::Long;

    r.entryPrice = t.entry;
    r.exitPrice = t.exit;          // persistiere immer; open-trades kann später NULL sein
    r.quantity = t.units;

    const int64_t now = nowUtcEpochSeconds();
    r.entryTime = now;
    r.exitTime = std::nullopt;

    r.venue = "";                  // später ggf. UI-Field
    r.comment = std::nullopt;

    r.createdAt = isInsert ? now : 0;
    r.updatedAt = now;

    return r;
}

static void applyTradeRow(Axiom::Trade& t, const axiom::db::TradeRow& r) {
    t.id = (int)r.id;
    t.setSymbol(r.symbol);
    t.entry = r.entryPrice;
    t.exit  = r.exitPrice.value_or(r.entryPrice);
    t.units = r.quantity;
}

// ---- AssetSpec Editor State (DB-backed) ----
struct AssetSpecEditorState {
    bool open = false;
    std::string symbol;
    axiom::db::AssetSpecRow working{};
    bool dirty = false;
};

static const char* assetTypeLabel(axiom::db::AssetType t) {
    switch (t) {
        case axiom::db::AssetType::FX:        return "FX";
        case axiom::db::AssetType::Commodity: return "Commodity";
        case axiom::db::AssetType::Crypto:    return "Crypto";
        default:                              return "CFD";
    }
}

static void openAssetEditor(AssetSpecEditorState& ed,
                            const std::string& symbol,
                            axiom::db::AssetSpecDao& dao)
{
    ed.open = true;
    ed.symbol = symbol;
    ed.dirty = false;

    if (auto spec = dao.getBySymbol(symbol)) {
        ed.working = *spec;
    } else {
        ed.working = defaultSpecForSymbol(symbol);
    }
}

static void drawAssetEditor(AssetSpecEditorState& ed,
                            axiom::db::AssetSpecDao& assetDao,
                            std::unordered_map<std::string, axiom::db::AssetSpecRow>& cache,
                            std::vector<Axiom::Trade>& trades)
{
    if (!ed.open) return;

    ImGui::OpenPopup("AssetSpec Editor");

    if (ImGui::BeginPopupModal("AssetSpec Editor", &ed.open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Symbol: %s", ed.symbol.c_str());
        ImGui::Separator();

        const char* types[] = { "FX", "Commodity", "Crypto", "CFD" };
        int typeIdx = (int)ed.working.assetType;
        if (ImGui::Combo("Asset Type##assetspec_type", &typeIdx, types, IM_ARRAYSIZE(types))) {
            ed.working.assetType = (axiom::db::AssetType)typeIdx;
            ed.dirty = true;

            // defaults on type switch
            if (ed.working.assetType == axiom::db::AssetType::FX) {
                ed.working.contractSize = 100000.0;
                ed.working.pipSize = 0.0001;
            } else {
                ed.working.contractSize = 1.0;
                ed.working.pipSize = 1.0;
            }
        }

        if (ImGui::InputDouble("Pip Size##assetspec_pip", &ed.working.pipSize, 0, 0, "%.6f")) {
            ed.dirty = true;
        }
        if (ImGui::InputDouble("Contract Size##assetspec_contract", &ed.working.contractSize, 0, 0, "%.2f")) {
            ed.dirty = true;
        }

        ImGui::SeparatorText("Presets");
        if (ImGui::Button("FX (0.0001 / 100000)##preset_fx")) {
            ed.working.assetType = axiom::db::AssetType::FX;
            ed.working.pipSize = 0.0001;
            ed.working.contractSize = 100000.0;
            ed.dirty = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("FX JPY (0.01 / 100000)##preset_fxjpy")) {
            ed.working.assetType = axiom::db::AssetType::FX;
            ed.working.pipSize = 0.01;
            ed.working.contractSize = 100000.0;
            ed.dirty = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Crypto (1 / 1)##preset_crypto")) {
            ed.working.assetType = axiom::db::AssetType::Crypto;
            ed.working.pipSize = 1.0;
            ed.working.contractSize = 1.0;
            ed.dirty = true;
        }

        ImGui::Separator();

        if (ImGui::Button("Cancel##assetspec_cancel")) {
            ed.open = false;
            ed.dirty = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        ImGui::BeginDisabled(!ed.dirty);
        if (ImGui::Button("Save##assetspec_save")) {
            // persist
            if (ed.working.createdAt == 0) ed.working.createdAt = nowUtcEpochSeconds();
            assetDao.upsert(ed.working); // uses upsert SQL in AssetSpecDao

            // update cache
            cache[ed.symbol] = ed.working;

            // apply to existing trades
            for (auto& t : trades) {
                if (t.symbol == ed.symbol) {
                    applySpecToTrade(t, ed.working);
                }
            }

            ed.open = false;
            ed.dirty = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        ImGui::EndPopup();
    }
}

int main(int argc, char** argv)
{
    (void)argc; (void)argv;

    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1200, 800, "AXIOM Trader - Stage D1", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 2;
    }

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 3;
    }

    id<MTLCommandQueue> commandQueue = [device newCommandQueue];

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplMetal_Init(device);

    NSWindow* nswin = glfwGetCocoaWindow(window);
    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = layer;
    nswin.contentView.wantsLayer = YES;

    // ---------------- DB init ----------------
    axiom::db::Connection conn("axiom.db");
    axiom::db::AssetSpecDao assetDao(conn);
    axiom::db::TradeDao tradeDao(conn);

    // create tables
    assetDao.ensureSchema();  // creates AssetSpec table
    tradeDao.ensureSchema();  // creates Trade table + indices

    // ---------------- Load DB state ----------------
    std::unordered_map<std::string, axiom::db::AssetSpecRow> assetCache;
    {
        // load all specs
        auto all = assetDao.getAll(); // present in AssetSpecDao implementation
        for (auto& s : all) assetCache[s.symbol] = s;
    }

    std::vector<Axiom::Trade> trades;
    {
        // load all trades
        auto rows = tradeDao.loadAll(); // present in TradeDao implementation
        trades.reserve(rows.size());
        for (const auto& r : rows) {
            Axiom::Trade t;
            applyTradeRow(t, r);

            // ensure spec exists
            if (assetCache.find(t.symbol) == assetCache.end()) {
                auto def = defaultSpecForSymbol(t.symbol);
                assetDao.upsert(def);
                assetCache[t.symbol] = def;
            }
            applySpecToTrade(t, assetCache[t.symbol]);
            trades.push_back(std::move(t));
        }
    }

    // ---------------- UI state ----------------
    int selectedIndex = -1;

    char assetBuf[32] = "EUR/USD";
    double entryBuf = 1.0800;
    double exitBuf  = 1.0855;
    double unitsBuf = 1.0;

    double pipValuePerLotUsd = 10.0;
    bool treatNonFxAsUnits = true;

    AssetSpecEditorState assetEditor;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int w = 0, h = 0;
        glfwGetFramebufferSize(window, &w, &h);
        layer.drawableSize = CGSizeMake(w, h);

        @autoreleasepool {
            id<CAMetalDrawable> drawable = [layer nextDrawable];
            if (!drawable) continue;

            MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];
            rp.colorAttachments[0].texture = drawable.texture;
            rp.colorAttachments[0].loadAction = MTLLoadActionClear;
            rp.colorAttachments[0].clearColor = MTLClearColorMake(0.02, 0.02, 0.03, 1.0);
            rp.colorAttachments[0].storeAction = MTLStoreActionStore;

            ImGui_ImplMetal_NewFrame(rp);
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("AXIOM Trader - Stage D1 (SQLite)");

            ImGui::SeparatorText("PnL Settings");
            ImGui::InputDouble("PipValue USD/Lot##pnl_pipvalue", &pipValuePerLotUsd);
            ImGui::Checkbox("Non-FX as Units##pnl_nonfx_units", &treatNonFxAsUnits);

            ImGui::SeparatorText("Trades (SQLite)");

            if (ImGui::BeginTable("Trades", 6,
                ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("ID");
                ImGui::TableSetupColumn("Asset");
                ImGui::TableSetupColumn("Type");
                ImGui::TableSetupColumn("Δ Price");
                ImGui::TableSetupColumn("Pips");
                ImGui::TableSetupColumn("Money");
                ImGui::TableHeadersRow();

                for (int i = 0; i < (int)trades.size(); ++i) {
                    ImGui::PushID(i);
                    auto& t = trades[i];
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", t.id);

                    ImGui::TableSetColumnIndex(1);
                    if (ImGui::Selectable(t.symbol.c_str(), selectedIndex == i)) {
                        selectedIndex = i;
                        snprintf(assetBuf, sizeof(assetBuf), "%s", t.symbol.c_str());
                        entryBuf = t.entry;
                        exitBuf  = t.exit;
                        unitsBuf = t.units;
                    }

                    ImGui::TableSetColumnIndex(2);
                    auto it = assetCache.find(t.symbol);
                    ImGui::Text("%s", (it != assetCache.end()) ? assetTypeLabel(it->second.assetType) : "Unspec");

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.4f", t.priceDelta());

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%.1f", t.calculatePips());

                    ImGui::TableSetColumnIndex(5);
                    ImGui::Text("%.2f", t.calculatePnL(pipValuePerLotUsd, treatNonFxAsUnits));

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }

            ImGui::SeparatorText("Trade Editor");

            ImGui::InputText("Asset##trade_asset", assetBuf, sizeof(assetBuf));
            ImGui::InputDouble("Entry##trade_entry", &entryBuf, 0, 0, "%.6f");
            ImGui::InputDouble("Exit##trade_exit", &exitBuf, 0, 0, "%.6f");
            ImGui::InputDouble("Units##trade_units", &unitsBuf);

            std::string curSym(assetBuf);

            // ensure AssetSpec exists (DB + cache)
            if (assetCache.find(curSym) == assetCache.end()) {
                auto def = defaultSpecForSymbol(curSym);
                assetDao.upsert(def);
                assetCache[curSym] = def;
            }

            auto& curSpec = assetCache[curSym];
            ImGui::TextDisabled("AssetSpec: %s | pip=%.6f | contract=%.2f",
                                assetTypeLabel(curSpec.assetType),
                                curSpec.pipSize,
                                curSpec.contractSize);

            if (ImGui::Button("Edit AssetSpec##open_assetspec")) {
                openAssetEditor(assetEditor, curSym, assetDao);
            }

            ImGui::SameLine();
            if (ImGui::Button("Add Trade##add_trade")) {
                Axiom::Trade t;
                t.id = -1;
                t.setSymbol(assetBuf);
                t.entry = entryBuf;
                t.exit  = exitBuf;
                t.units = unitsBuf;

                applySpecToTrade(t, curSpec);

                // persist trade
                auto row = toTradeRow(t, true);
                int64_t newId = tradeDao.insert(row); // TradeDao::insert exists
                t.id = (int)newId;

                trades.push_back(std::move(t));
                selectedIndex = -1;
            }

            ImGui::SameLine();
            ImGui::BeginDisabled(selectedIndex < 0);
            if (ImGui::Button("Update Selected##update_trade")) {
                auto& t = trades[selectedIndex];
                t.setSymbol(assetBuf);
                t.entry = entryBuf;
                t.exit  = exitBuf;
                t.units = unitsBuf;

                // ensure spec exists
                if (assetCache.find(t.symbol) == assetCache.end()) {
                    auto def = defaultSpecForSymbol(t.symbol);
                    assetDao.upsert(def);
                    assetCache[t.symbol] = def;
                }
                applySpecToTrade(t, assetCache[t.symbol]);

                // persist update
                auto row = toTradeRow(t, false);
                row.id = t.id; // required by update WHERE id=?
                tradeDao.update(row); // TradeDao::update exists
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete Selected##delete_trade")) {
                int delId = trades[selectedIndex].id;
                tradeDao.removeById(delId); // TradeDao::removeById exists
                trades.erase(trades.begin() + selectedIndex);
                selectedIndex = -1;
            }
            ImGui::EndDisabled();

            ImGui::SeparatorText("Asset Registry (SQLite cached)");
            if (ImGui::BeginTable("AssetRegistry", 4,
                ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("Symbol");
                ImGui::TableSetupColumn("Type");
                ImGui::TableSetupColumn("Pip");
                ImGui::TableSetupColumn("Contract");
                ImGui::TableHeadersRow();

                for (const auto& kv : assetCache) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", kv.first.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s%s", assetTypeLabel(kv.second.assetType), isJpy(kv.second) ? " (JPY)" : "");
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.6f", kv.second.pipSize);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.2f", kv.second.contractSize);
                }

                ImGui::EndTable();
            }

            // draw modal last
            drawAssetEditor(assetEditor, assetDao, assetCache, trades);

            ImGui::End();

            id<MTLCommandBuffer> cb = [commandQueue commandBuffer];
            id<MTLRenderCommandEncoder> ce =
                [cb renderCommandEncoderWithDescriptor:rp];

            ImGui::Render();
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cb, ce);

            [ce endEncoding];
            [cb presentDrawable:drawable];
            [cb commit];
        }
    }

    ImGui_ImplMetal_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

