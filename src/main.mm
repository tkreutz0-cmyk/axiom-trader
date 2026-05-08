#include <stdio.h>
#include <string>
#include <vector>
#include <chrono>
#include <map>
#include <cmath>

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
    int id; std::string symbol; TradeSide side; double entryPrice; double exitPrice;
    std::chrono::system_clock::time_point timestamp;
    double getPnL() const { return (side == LONG) ? (exitPrice - entryPrice) : (entryPrice - exitPrice); }
};

// --- HILFSFUNKTIONEN ---
ImU32 GetAssetColor(std::string s, float alpha = 1.0f) {
    size_t hash = std::hash<std::string>{}(s);
    float r = ((hash & 0xFF0000) >> 16) / 255.0f;
    float g = ((hash & 0x00FF00) >> 8) / 255.0f;
    float b = (hash & 0x0000FF) / 255.0f;
    return ImGui::ColorConvertFloat4ToU32(ImVec4(r * 0.7f + 0.3f, g * 0.7f + 0.3f, b * 0.7f + 0.3f, alpha));
}

struct GeoPos { float lon; float lat; };
std::map<std::string, GeoPos> AssetLocations = {
    {"BTC/USD", {-74.0, 40.7}}, {"EUR/USD", {8.6, 50.1}}, {"GBP/USD", {-0.1, 51.5}}, {"USD/JPY", {139.6, 35.6}}, {"AUD/USD", {151.2, -33.8}}
};

void RenderAxiomWorld(ImDrawList* dl, ImVec2 p, ImVec2 s, std::vector<AxiomTrade>& trades, int& sel_id, ImTextureID mapTex) {
    const float aspect = 2.0f;
    ImVec2 mS = s;
    if ((mS.x / mS.y) > aspect) mS.x = mS.y * aspect; else mS.y = mS.x / aspect;
    ImVec2 mP = ImVec2(p.x + (s.x - mS.x) * 0.5f, p.y + (s.y - mS.y) * 0.5f);
    
    dl->AddRectFilled(mP, ImVec2(mP.x + mS.x, mP.y + mS.y), IM_COL32(10, 10, 15, 255), 4.0f);
    if (mapTex != 0) dl->AddImage(mapTex, mP, ImVec2(mP.x + mS.x, mP.y + mS.y));
    else dl->AddRect(mP, ImVec2(mP.x + mS.x, mP.y + mS.y), IM_COL32(30, 30, 45, 255), 4.0f);

    dl->PushClipRect(mP, ImVec2(mP.x + mS.x, mP.y + mS.y), true);
    auto MapPos = [&](float lon, float lat) { return ImVec2(mP.x + (lon + 180.f) / 360.f * mS.x, mP.y + (90.f - lat) / 180.f * mS.y); };

    ImVec2 mousePos = ImGui::GetMousePos();
    bool mouseClicked = ImGui::IsMouseClicked(0);

    for(const auto& t : trades) {
        GeoPos gPos = {0, 20}; if (AssetLocations.count(t.symbol)) gPos = AssetLocations[t.symbol];
        ImVec2 screenPos = MapPos(gPos.lon, gPos.lat);
        
        float dist = sqrtf(powf(mousePos.x - screenPos.x, 2) + powf(mousePos.y - screenPos.y, 2));
        if (mouseClicked && dist < 15.0f) sel_id = t.id;

        bool is_sel = (t.id == sel_id);
        ImU32 assetCol = GetAssetColor(t.symbol, is_sel ? 1.0f : 0.7f);
        
        if (t.side == LONG) dl->AddTriangleFilled(ImVec2(screenPos.x, screenPos.y - 6), ImVec2(screenPos.x - 5, screenPos.y + 3), ImVec2(screenPos.x + 5, screenPos.y + 3), assetCol);
        else dl->AddTriangleFilled(ImVec2(screenPos.x, screenPos.y + 6), ImVec2(screenPos.x - 5, screenPos.y - 3), ImVec2(screenPos.x + 5, screenPos.y - 3), assetCol);
        
        if(is_sel) dl->AddCircle(screenPos, 12.0f, IM_COL32_WHITE, 16, 1.5f);
        
        char label[128];
        snprintf(label, 128, "%s %s", (t.side == LONG ? "[L]" : "[S]"), t.symbol.c_str());
        dl->AddText(ImVec2(screenPos.x + 12, screenPos.y - 8), is_sel ? IM_COL32_WHITE : assetCol, label);
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
    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOther(window, true); ImGui_ImplMetal_Init(device);
    NSWindow* nswin = glfwGetCocoaWindow(window);
    CAMetalLayer* layer = [CAMetalLayer layer]; layer.device = device; layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = layer; nswin.contentView.wantsLayer = YES;

    std::vector<AxiomTrade> my_trades;
    int next_id = 0, selected_id = -1, last_selected = -1;
    ImTextureID dummyTex = 0;
    char asset_buf[64] = "BTC/USD";
    double entry_val = 60000.0, exit_val = 61000.0; int side_idx = 0;
    bool layout_init_needed = true;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        int w, h; glfwGetFramebufferSize(window, &w, &h); layer.drawableSize = CGSizeMake(w, h);
        @autoreleasepool {
            id<CAMetalDrawable> drawable = [layer nextDrawable]; if (!drawable) continue;
            MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];
            rp.colorAttachments[0].texture = drawable.texture;
            rp.colorAttachments[0].loadAction = MTLLoadActionClear;
            rp.colorAttachments[0].clearColor = MTLClearColorMake(0.01, 0.01, 0.015, 1.0);
            rp.colorAttachments[0].storeAction = MTLStoreActionStore;

            ImGui_ImplMetal_NewFrame(rp); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();

            // --- SYSTEM: LAYOUT MANAGER ---
            ImGui::Begin("SYSTEM: Layout");
            if (ImGui::Button("Reset to Standard Grid", ImVec2(-1, 30)) || layout_init_needed) {
                ImGui::SetWindowPos("SYNAPSE Master-Control", ImVec2(20, 20));
                ImGui::SetWindowSize("SYNAPSE Master-Control", ImVec2(350, 500));
                ImGui::SetWindowPos("World Monitor", ImVec2(380, 20));
                ImGui::SetWindowSize("World Monitor", ImVec2(1040, 600));
                ImGui::SetWindowPos("SYSTEM: Layout", ImVec2(20, 530));
                ImGui::SetWindowSize("SYSTEM: Layout", ImVec2(350, 100));
                layout_init_needed = false;
            }
            ImGui::End();

            // --- MASTER CONTROL ---
            ImGui::Begin("SYNAPSE Master-Control");
            if (selected_id != -1 && selected_id != last_selected) {
                for(auto& t : my_trades) if(t.id == selected_id) {
                    strncpy(asset_buf, t.symbol.c_str(), 64);
                    entry_val = t.entryPrice; exit_val = t.exitPrice; side_idx = (int)t.side;
                    break;
                }
                last_selected = selected_id;
            }

            ImGui::InputText("Asset", asset_buf, 64);
            const char* sides[] = { "LONG", "SHORT" }; ImGui::Combo("Side", &side_idx, sides, 2);
            ImGui::InputDouble("In", &entry_val); ImGui::InputDouble("Out", &exit_val);
            
            if (selected_id == -1) {
                if (ImGui::Button("Add Trade", ImVec2(-1, 30)))
                    my_trades.push_back({next_id++, std::string(asset_buf), (TradeSide)side_idx, entry_val, exit_val, std::chrono::system_clock::now()});
            } else {
                if (ImGui::Button("Update", ImVec2(100, 30))) {
                    for(auto& t : my_trades) if(t.id == selected_id) { t.symbol = asset_buf; t.entryPrice = entry_val; t.exitPrice = exit_val; t.side = (TradeSide)side_idx; }
                }
                ImGui::SameLine(); if (ImGui::Button("Deselect", ImVec2(-1, 30))) { selected_id = -1; last_selected = -1; }
            }

            ImGui::Separator();
            if (ImGui::BeginTable("Trades", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Asset"); ImGui::TableSetupColumn("PnL"); ImGui::TableSetupColumn("Action"); ImGui::TableHeadersRow();
                for (auto it = my_trades.begin(); it != my_trades.end(); ) {
                    ImGui::PushID(it->id); ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
                    char t_label[128]; snprintf(t_label, 128, "%s %s", (it->side == LONG ? "▲" : "▼"), it->symbol.c_str());
                    if (ImGui::Selectable(t_label, selected_id == it->id)) selected_id = it->id;
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextColored(it->getPnL() >= 0 ? ImVec4(0.4,1,0,1) : ImVec4(1,0.4,0,1), "%.2f", it->getPnL());
                    ImGui::TableSetColumnIndex(2);
                    if (ImGui::SmallButton("DEL")) { if (selected_id == it->id) selected_id = -1; it = my_trades.erase(it); } else { ++it; }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::End();

            // --- WORLD MONITOR ---
            ImGui::Begin("World Monitor");
            RenderAxiomWorld(ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail(), my_trades, selected_id, dummyTex);
            ImGui::End();

            id<MTLCommandBuffer> cb = [commandQueue commandBuffer];
            id<MTLRenderCommandEncoder> ce = [cb renderCommandEncoderWithDescriptor:rp];
            ImGui::Render(); ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cb, ce);
            [ce endEncoding]; [cb presentDrawable:drawable]; [cb commit];
        }
    }
    return 0;
}


