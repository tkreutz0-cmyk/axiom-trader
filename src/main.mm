#include <stdio.h>
#include <string>
#include <vector>
#include <chrono>
#include <map>
#include <cmath>
#include <algorithm>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

// --- DATENSTRUKTUR ---
enum TradeSide { LONG = 0, SHORT = 1 };

struct AxiomTrade {
    int id;
    std::string symbol;
    TradeSide side;
    double entryPrice;
    double exitPrice;
    std::chrono::system_clock::time_point timestamp;

    // 1) Preis-Delta (reiner Kursunterschied)
    double getPnLPriceDelta() const {
        return (side == LONG) ? (exitPrice - entryPrice) : (entryPrice - exitPrice);
    }

    // Heuristik: FX erkennen (z.B. "EUR/USD", "USD/JPY")
    bool isFX() const {
        auto pos = symbol.find('/');
        if (pos == std::string::npos) return false;
        return (pos == 3 && symbol.size() >= 7);
    }

    // Quote currency = Teil nach '/'
    bool isJPYQuote() const {
        auto pos = symbol.find('/');
        if (pos == std::string::npos) return false;
        std::string quote = symbol.substr(pos + 1);
        return (quote == "JPY");
    }

    // 2) Pips (nur sinnvoll bei FX, bei Nicht-FX fällt es auf Delta zurück)
    double getPnLPips() const {
        double delta = getPnLPriceDelta();
        if (!isFX()) return delta; // fallback
        double pipFactor = isJPYQuote() ? 100.0 : 10000.0;
        return delta * pipFactor;
    }

    // 3) Geld-PnL (Heuristik/Parameter)
    // - FX: Money = Pips * PipValuePerLotUSD * Lots
    // - Non-FX: Money = Delta * Units (wenn treatNonFXasUnits)
    double getPnLMoney(double lots_or_units,
                      double pipValuePerLotUSD,
                      bool treatNonFXasUnits) const
    {
        double delta = getPnLPriceDelta();

        if (isFX()) {
            double pips = getPnLPips();
            return pips * pipValuePerLotUSD * lots_or_units;
        }

        if (treatNonFXasUnits) return delta * lots_or_units;
        return delta;
    }
};

// --- HILFSFUNKTIONEN ---
ImU32 GetAssetColor(std::string s, float alpha = 1.0f) {
    size_t hash = std::hash<std::string>{}(s);
    float r = ((hash & 0xFF0000) >> 16) / 255.0f;
    float g = ((hash & 0x00FF00) >> 8) / 255.0f;
    float b = (hash & 0x0000FF) / 255.0f;
    return ImGui::ColorConvertFloat4ToU32(
        ImVec4(r * 0.7f + 0.3f, g * 0.7f + 0.3f, b * 0.7f + 0.3f, alpha)
    );
}

struct GeoPos { float lon; float lat; };
std::map<std::string, GeoPos> AssetLocations = {
    {"BTC/USD", {-74.0f, 40.7f}},
    {"EUR/USD", {8.6f, 50.1f}},
    {"GBP/USD", {-0.1f, 51.5f}},
    {"USD/JPY", {139.6f, 35.6f}},
    {"AUD/USD", {151.2f, -33.8f}}
};

enum PnLDisplayMode {
    PNL_PRICE_DELTA = 0,
    PNL_PIPS        = 1,
    PNL_MONEY       = 2
};

static void FormatPnLLabel(const AxiomTrade& t,
                           int pnl_mode,
                           double lots_or_units,
                           double pip_value_per_lot_usd,
                           bool treat_nonfx_as_units,
                           char* out,
                           size_t out_sz)
{
    if (pnl_mode == PNL_PRICE_DELTA) {
        double v = t.getPnLPriceDelta();
        if (t.isFX()) snprintf(out, out_sz, "Δ %.5f", v);
        else         snprintf(out, out_sz, "Δ %.2f", v);
    } else if (pnl_mode == PNL_PIPS) {
        double v = t.getPnLPips();
        if (t.isFX()) snprintf(out, out_sz, "%.1f pips", v);
        else         snprintf(out, out_sz, "%.5f", v);
    } else { // money
        double v = t.getPnLMoney(lots_or_units, pip_value_per_lot_usd, treat_nonfx_as_units);
        snprintf(out, out_sz, "%.2f", v);
    }
}

// --- WORLD RENDER: CLUSTER ---
void RenderAxiomWorld(ImDrawList* dl,
                      ImVec2 p, ImVec2 s,
                      std::vector<AxiomTrade>& trades,
                      int& sel_id,
                      ImTextureID mapTex,
                      int pnl_mode,
                      double lots_or_units,
                      double pip_value_per_lot_usd,
                      bool treat_nonfx_as_units)
{
    const float aspect = 2.0f;
    ImVec2 mS = s;
    if ((mS.x / mS.y) > aspect) mS.x = mS.y * aspect;
    else mS.y = mS.x / aspect;
    ImVec2 mP = ImVec2(p.x + (s.x - mS.x) * 0.5f, p.y + (s.y - mS.y) * 0.5f);

    dl->AddRectFilled(mP, ImVec2(mP.x + mS.x, mP.y + mS.y), IM_COL32(10, 10, 15, 255), 4.0f);
    if (mapTex != 0) dl->AddImage(mapTex, mP, ImVec2(mP.x + mS.x, mP.y + mS.y));
    else dl->AddRect(mP, ImVec2(mP.x + mS.x, mP.y + mS.y), IM_COL32(30, 30, 45, 255), 4.0f);

    dl->PushClipRect(mP, ImVec2(mP.x + mS.x, mP.y + mS.y), true);

    auto MapPos = [&](float lon, float lat) {
        return ImVec2(mP.x + (lon + 180.f) / 360.f * mS.x,
                      mP.y + (90.f - lat) / 180.f * mS.y);
    };

    ImVec2 mousePos = ImGui::GetMousePos();
    bool mouseClicked = ImGui::IsMouseClicked(0);

    bool mouseInMap =
        (mousePos.x >= mP.x && mousePos.x <= mP.x + mS.x &&
         mousePos.y >= mP.y && mousePos.y <= mP.y + mS.y);

    // --- Cluster pro Symbol ---
    struct Cluster {
        std::string symbol;
        GeoPos gpos;
        std::vector<int> idx; // indices into trades
        int longCount = 0;
        int shortCount = 0;
    };
    std::map<std::string, Cluster> clusters;
    clusters.clear();

    for (int i = 0; i < (int)trades.size(); ++i) {
        const auto& t = trades[i];
        if (!clusters.count(t.symbol)) {
            Cluster c;
            c.symbol = t.symbol;
            c.gpos = {0, 20};
            if (AssetLocations.count(t.symbol)) c.gpos = AssetLocations[t.symbol];
            clusters[t.symbol] = c;
        }
        clusters[t.symbol].idx.push_back(i);
        if (trades[i].side == LONG) clusters[t.symbol].longCount++;
        else clusters[t.symbol].shortCount++;
    }

    // Popup State (bleibt über Frames erhalten)
    static std::string active_cluster_symbol;
    static ImVec2 active_cluster_anchor = ImVec2(0, 0);

    // --- Click-Handling: erst den nächsten Cluster/Marker finden ---
    if (mouseClicked && mouseInMap) {
        float bestDist = 1e9f;
        bool bestWasCluster = false;
        std::string bestClusterSym;
        int bestTradeId = -1;

        for (auto& kv : clusters) {
            const Cluster& c = kv.second;
            ImVec2 base = MapPos(c.gpos.lon, c.gpos.lat);

            const int count = (int)c.idx.size();
            const float clusterRadius = (count <= 1) ? 15.0f : (16.0f + std::min(14.0f, count * 0.8f));

            float dx = mousePos.x - base.x;
            float dy = mousePos.y - base.y;
            float dist = sqrtf(dx*dx + dy*dy);

            if (dist < clusterRadius && dist < bestDist) {
                bestDist = dist;
                if (count > 1) {
                    bestWasCluster = true;
                    bestClusterSym = c.symbol;
                    bestTradeId = -1;
                } else {
                    bestWasCluster = false;
                    bestTradeId = trades[c.idx[0]].id;
                    bestClusterSym.clear();
                }
            }
        }

        if (bestDist < 1e8f) {
            if (bestWasCluster) {
                active_cluster_symbol = bestClusterSym;
                // Anchor position: cluster base pos
                GeoPos gp = clusters[active_cluster_symbol].gpos;
                active_cluster_anchor = MapPos(gp.lon, gp.lat);
                ImGui::OpenPopup("Cluster Trades");
            } else if (bestTradeId != -1) {
                sel_id = bestTradeId;
                active_cluster_symbol.clear();
            }
        }
    }

    // --- Render Clusters/Markers ---
    for (auto& kv : clusters) {
        const Cluster& c = kv.second;
        ImVec2 base = MapPos(c.gpos.lon, c.gpos.lat);
        const int count = (int)c.idx.size();

        // Gibt es einen selektierten Trade in diesem Cluster?
        bool cluster_has_selected = false;
        for (int ti : c.idx) {
            if (trades[ti].id == sel_id) { cluster_has_selected = true; break; }
        }

        ImU32 col = GetAssetColor(c.symbol, count > 1 ? 0.85f : 0.75f);

        if (count == 1) {
            const AxiomTrade& t = trades[c.idx[0]];
            if (t.side == LONG) {
                dl->AddTriangleFilled(ImVec2(base.x, base.y - 6),
                                      ImVec2(base.x - 5, base.y + 3),
                                      ImVec2(base.x + 5, base.y + 3),
                                      col);
            } else {
                dl->AddTriangleFilled(ImVec2(base.x, base.y + 6),
                                      ImVec2(base.x - 5, base.y - 3),
                                      ImVec2(base.x + 5, base.y - 3),
                                      col);
            }
            if (cluster_has_selected) dl->AddCircle(base, 12.0f, IM_COL32_WHITE, 16, 1.5f);

            char label[128];
            snprintf(label, sizeof(label), "%s %s", (t.side == LONG ? "[L]" : "[S]"), t.symbol.c_str());
            dl->AddText(ImVec2(base.x + 12, base.y - 8), cluster_has_selected ? IM_COL32_WHITE : col, label);
        } else {
            // Cluster-Kreis
            float radius = 16.0f + std::min(14.0f, count * 0.8f);
            dl->AddCircleFilled(base, radius, col, 24);

            // Outline
            dl->AddCircle(base, radius, cluster_has_selected ? IM_COL32(255,255,255,220) : IM_COL32(10,10,15,220), 24, 2.0f);

            // Count text (zentriert)
            char cnt[16];
            snprintf(cnt, sizeof(cnt), "%d", count);
            ImVec2 tsz = ImGui::CalcTextSize(cnt);
            dl->AddText(ImVec2(base.x - tsz.x * 0.5f, base.y - tsz.y * 0.5f),
                        IM_COL32(255,255,255,230), cnt);

            // Mini Hinweis (LONG/SHORT Mix) als Tooltip
            // Tooltip via ImGui (nur wenn Maus drüber)
            float dx = mousePos.x - base.x;
            float dy = mousePos.y - base.y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist < radius && mouseInMap) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", c.symbol.c_str());
                ImGui::Separator();
                ImGui::Text("Trades: %d", count);
                ImGui::Text("LONG: %d  SHORT: %d", c.longCount, c.shortCount);
                ImGui::TextDisabled("Klick: Auswahl-Liste");
                ImGui::EndTooltip();
            }
        }
    }

    // --- Popup: Trades im Cluster auswählen ---
    if (ImGui::BeginPopup("Cluster Trades")) {
        // Popup nahe am Cluster platzieren
        ImGui::SetWindowFontScale(1.0f);

        // falls Symbol nicht mehr existiert (z.B. gelöscht), Popup schließen
        if (!active_cluster_symbol.empty() && clusters.count(active_cluster_symbol)) {
            const Cluster& c = clusters[active_cluster_symbol];

            // Position: leicht neben Anchor
            ImGui::SetCursorPosX(0);
            ImGui::Text("%s", active_cluster_symbol.c_str());
            ImGui::Separator();

            for (int ti : c.idx) {
                const AxiomTrade& t = trades[ti];
                char pnl[64];
                FormatPnLLabel(t, pnl_mode, lots_or_units, pip_value_per_lot_usd, treat_nonfx_as_units, pnl, sizeof(pnl));

                char line[256];
                snprintf(line, sizeof(line),
                         "#%d  %s  In %.6f  Out %.6f   (%s)",
                         t.id,
                         (t.side == LONG ? "LONG" : "SHORT"),
                         t.entryPrice, t.exitPrice,
                         pnl);

                if (ImGui::Selectable(line, sel_id == t.id)) {
                    sel_id = t.id;
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::Separator();
            if (ImGui::Button("Close", ImVec2(-1, 0))) {
                ImGui::CloseCurrentPopup();
            }
        } else {
            ImGui::TextDisabled("Cluster nicht mehr vorhanden.");
            if (ImGui::Button("Close", ImVec2(-1, 0))) ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    dl->PopClipRect();
}

int main(int, char**) {
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1440, 900, "AXIOM Trader v43.0", nullptr, nullptr);

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    id<MTLCommandQueue> commandQueue = [device newCommandQueue];

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplMetal_Init(device);

    NSWindow* nswin = glfwGetCocoaWindow(window);
    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = layer;
    nswin.contentView.wantsLayer = YES;

    std::vector<AxiomTrade> my_trades;
    int next_id = 0, selected_id = -1, last_selected = -1;
    ImTextureID dummyTex = 0;

    char asset_buf[64] = "BTC/USD";
    double entry_val = 60000.0, exit_val = 61000.0;
    int side_idx = 0;

    bool layout_init_needed = true;

    // --- PnL Anzeige-/Berechnungsparameter ---
    int pnl_mode = PNL_PIPS;                 // default: pips
    double lots_or_units = 1.0;              // FX: Lots, Non-FX: Units
    double pip_value_per_lot_usd = 10.0;     // grob: 10 USD/Pip/Lot bei XXX/USD
    bool treat_nonfx_as_units = true;        // Non-FX: Delta * Units

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        layer.drawableSize = CGSizeMake(w, h);

        @autoreleasepool {
            id<CAMetalDrawable> drawable = [layer nextDrawable];
            if (!drawable) continue;

            MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];
            rp.colorAttachments[0].texture = drawable.texture;
            rp.colorAttachments[0].loadAction = MTLLoadActionClear;
            rp.colorAttachments[0].clearColor = MTLClearColorMake(0.01, 0.01, 0.015, 1.0);
            rp.colorAttachments[0].storeAction = MTLStoreActionStore;

            ImGui_ImplMetal_NewFrame(rp);
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // --- SYSTEM: LAYOUT MANAGER ---
            ImGui::Begin("SYSTEM: Layout");
            if (ImGui::Button("Reset to Standard Grid", ImVec2(-1, 30)) || layout_init_needed) {
                ImGui::SetWindowPos("SYNAPSE Master-Control", ImVec2(20, 20));
                ImGui::SetWindowSize("SYNAPSE Master-Control", ImVec2(350, 560));
                ImGui::SetWindowPos("World Monitor", ImVec2(380, 20));
                ImGui::SetWindowSize("World Monitor", ImVec2(1040, 600));
                ImGui::SetWindowPos("SYSTEM: Layout", ImVec2(20, 590));
                ImGui::SetWindowSize("SYSTEM: Layout", ImVec2(350, 100));
                layout_init_needed = false;
            }
            ImGui::End();

            // --- MASTER CONTROL ---
            ImGui::Begin("SYNAPSE Master-Control");

            if (selected_id != -1 && selected_id != last_selected) {
                for (auto& t : my_trades) if (t.id == selected_id) {
                    snprintf(asset_buf, sizeof(asset_buf), "%s", t.symbol.c_str()); // robust
                    entry_val = t.entryPrice;
                    exit_val  = t.exitPrice;
                    side_idx  = (int)t.side;
                    break;
                }
                last_selected = selected_id;
            }

            ImGui::InputText("Asset", asset_buf, sizeof(asset_buf));
            const char* sides[] = { "LONG", "SHORT" };
            ImGui::Combo("Side", &side_idx, sides, 2);
            ImGui::InputDouble("In", &entry_val);
            ImGui::InputDouble("Out", &exit_val);

            if (selected_id == -1) {
                if (ImGui::Button("Add Trade", ImVec2(-1, 30))) {
                    my_trades.push_back({
                        next_id++,
                        std::string(asset_buf),
                        (TradeSide)side_idx,
                        entry_val,
                        exit_val,
                        std::chrono::system_clock::now()
                    });
                }
            } else {
                if (ImGui::Button("Update", ImVec2(100, 30))) {
                    for (auto& t : my_trades) if (t.id == selected_id) {
                        t.symbol = asset_buf;
                        t.entryPrice = entry_val;
                        t.exitPrice  = exit_val;
                        t.side = (TradeSide)side_idx;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Deselect", ImVec2(-1, 30))) {
                    selected_id = -1;
                    last_selected = -1;
                }
            }

            ImGui::SeparatorText("PnL Anzeige");
            const char* pnl_modes[] = { "Preis-Delta", "Pips (FX)", "Geld (Lots/Units)" };
            ImGui::Combo("Mode", &pnl_mode, pnl_modes, 3);
            ImGui::InputDouble("Lots/Units", &lots_or_units);
            ImGui::InputDouble("Pip-Value $/Lot", &pip_value_per_lot_usd);
            ImGui::Checkbox("Non-FX als Units (Delta*Units)", &treat_nonfx_as_units);

            ImGui::Separator();

            if (ImGui::BeginTable("Trades", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Asset");
                ImGui::TableSetupColumn("PnL");
                ImGui::TableSetupColumn("Action");
                ImGui::TableHeadersRow();

                for (auto it = my_trades.begin(); it != my_trades.end(); ) {
                    ImGui::PushID(it->id);
                    ImGui::TableNextRow();

                    // Asset
                    ImGui::TableSetColumnIndex(0);
                    char t_label[128];
                    snprintf(t_label, sizeof(t_label), "%s %s", (it->side == LONG ? "▲" : "▼"), it->symbol.c_str());
                    if (ImGui::Selectable(t_label, selected_id == it->id)) selected_id = it->id;

                    // PnL
                    ImGui::TableSetColumnIndex(1);
                    ImVec4 col = (it->getPnLPriceDelta() >= 0.0) ? ImVec4(0.4f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.4f, 0.0f, 1.0f);

                    if (pnl_mode == PNL_PRICE_DELTA) {
                        double v = it->getPnLPriceDelta();
                        if (it->isFX()) ImGui::TextColored(col, "%.5f", v);
                        else            ImGui::TextColored(col, "%.2f", v);
                    }
                    else if (pnl_mode == PNL_PIPS) {
                        double v = it->getPnLPips();
                        if (it->isFX()) ImGui::TextColored(col, "%.1f pips", v);
                        else            ImGui::TextColored(col, "%.5f", v);
                    }
                    else { // money
                        double v = it->getPnLMoney(lots_or_units, pip_value_per_lot_usd, treat_nonfx_as_units);
                        ImGui::TextColored(col, "%.2f", v);
                        ImGui::SameLine();
                        ImGui::TextDisabled("%s", (it->isFX() ? "USD (heur.)" : (treat_nonfx_as_units ? "(Delta*Units)" : "(Delta)")));
                    }

                    // Action
                    ImGui::TableSetColumnIndex(2);
                    if (ImGui::SmallButton("DEL")) {
                        if (selected_id == it->id) selected_id = -1;
                        it = my_trades.erase(it);
                    } else {
                        ++it;
                    }

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            ImGui::End();

            // --- WORLD MONITOR ---
            ImGui::Begin("World Monitor");
            RenderAxiomWorld(ImGui::GetWindowDrawList(),
                             ImGui::GetCursorScreenPos(),
                             ImGui::GetContentRegionAvail(),
                             my_trades,
                             selected_id,
                             dummyTex,
                             pnl_mode,
                             lots_or_units,
                             pip_value_per_lot_usd,
                             treat_nonfx_as_units);
            ImGui::End();

            id<MTLCommandBuffer> cb = [commandQueue commandBuffer];
            id<MTLRenderCommandEncoder> ce = [cb renderCommandEncoderWithDescriptor:rp];

            ImGui::Render();
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cb, ce);

            [ce endEncoding];
            [cb presentDrawable:drawable];
            [cb commit];
        }
    }

    return 0;
}
