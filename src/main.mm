#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <ctime>

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

enum class TradeStatus { PLANNED = 0, REALIZED = 1 };
struct AxiomTrade {
    int id; std::string symbol; double entryPrice; double exitPrice;
    std::chrono::system_clock::time_point timestamp;
    TradeStatus status;
};

/* ==========================================================
   GEOMETRY ENGINE: MATH & TRIANGULATION
   ========================================================== */
static float Cross(const ImVec2& a, const ImVec2& b, const ImVec2& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static bool PointInTri(const ImVec2& p, const ImVec2& a, const ImVec2& b, const ImVec2& c) {
    float c1 = Cross(a, b, p); float c2 = Cross(b, c, p); float c3 = Cross(c, a, p);
    return !(((c1 < 0) || (c2 < 0) || (c3 < 0)) && ((c1 > 0) || (c2 > 0) || (c3 > 0)));
}

static void NormalizePoly(std::vector<ImVec2>& p) {
    if (p.size() >= 2) {
        if (fabsf(p.front().x - p.back().x) < 0.001f && fabsf(p.front().y - p.back().y) < 0.001f) p.pop_back();
    }
    if (p.size() < 3) return;
    double area = 0.0;
    for (int i = 0; i < (int)p.size(); ++i) area += (double)p[i].x * p[(i + 1) % p.size()].y - (double)p[(i + 1) % p.size()].x * p[i].y;
    if (area < 0.0) std::reverse(p.begin(), p.end());
}

static void FillConcavePoly(ImDrawList* dl, const std::vector<ImVec2>& inPoly, ImU32 col) {
    if (inPoly.size() < 3) return;
    std::vector<ImVec2> poly = inPoly; NormalizePoly(poly);
    std::vector<int> idx(poly.size());
    for (int i = 0; i < (int)idx.size(); ++i) idx[i] = i;
    int guard = 0;
    while (idx.size() > 2 && guard++ < 1000) {
        bool clipped = false;
        for (int i = 0; i < (int)idx.size(); ++i) {
            int i0 = idx[(i + (int)idx.size() - 1) % (int)idx.size()], i1 = idx[i], i2 = idx[(i + 1) % (int)idx.size()];
            if (Cross(poly[i0], poly[i1], poly[i2]) <= 0.0f) continue;
            bool anyIn = false;
            for (int j = 0; j < (int)idx.size(); j++) {
                if (idx[j] == i0 || idx[j] == i1 || idx[j] == i2) continue;
                if (PointInTri(poly[idx[j]], poly[i0], poly[i1], poly[i2])) { anyIn = true; break; }
            }
            if (anyIn) continue;
            dl->AddTriangleFilled(poly[i0], poly[i1], poly[i2], col);
            idx.erase(idx.begin() + i); clipped = true; break;
        }
        if (!clipped) break;
    }
}

/* ==========================================================
   AXIOM WORLD ENGINE
   ========================================================== */
void RenderAxiomWorld(ImDrawList* dl, ImVec2 p, ImVec2 s, const std::vector<AxiomTrade>& trades, int sel_id) {
    const float targetAspect = 2.0f;
    ImVec2 mS = s;
    if ((mS.x / mS.y) > targetAspect) mS.x = mS.y * targetAspect;
    else mS.y = mS.x / targetAspect;
    ImVec2 mP = ImVec2(p.x + (s.x - mS.x) * 0.5f, p.y + (s.y - mS.y) * 0.5f);
    
    dl->AddRectFilled(mP, ImVec2(mP.x + mS.x, mP.y + mS.y), IM_COL32(10, 10, 15, 255), 4.0f);
    dl->PushClipRect(mP, ImVec2(mP.x + mS.x, mP.y + mS.y), true);

    auto Map = [&](float lon, float lat) {
        return ImVec2(mP.x + (lon + 180.f) / 360.f * mS.x, mP.y + (90.f - lat) / 180.f * mS.y);
    };

    std::vector<std::vector<ImVec2>> world = {
        {{-168,65},{-120,70},{-70,72},{-55,50},{-80,15},{-120,30},{-168,65}},
        {{-80,12},{-50,10},{-40,-20},{-60,-55},{-82,-20},{-80,12}},
        {{-10,35},{15,45},{30,65},{60,75},{120,70},{150,60},{140,10},{50,5},{-10,35}}
    };

    for (auto& poly : world) {
        std::vector<ImVec2> sPts;
        for (auto v : poly) sPts.push_back(Map(v.x, v.y));
        FillConcavePoly(dl, sPts, IM_COL32(40, 45, 65, 255));
        dl->AddPolyline(sPts.data(), (int)sPts.size(), IM_COL32(110, 130, 170, 255), ImDrawFlags_Closed, 1.2f);
    }

    for(const auto& t : trades) {
        time_t tt = std::chrono::system_clock::to_time_t(t.timestamp);
        struct tm* tu = gmtime(&tt);
        float lon = (tu->tm_hour / 24.0f) * 360.0f - 180.0f;
        dl->AddCircleFilled(Map(lon, 0), 6, (t.id == sel_id) ? IM_COL32_WHITE : IM_COL32(0, 255, 255, 200));
    }
    dl->PopClipRect();
}

int main(int, char**) {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1440, 900, "AXIOM Trader v35.0", nullptr, nullptr);

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    id<MTLCommandQueue> commandQueue = [device newCommandQueue];
    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOther(window, true); ImGui_ImplMetal_Init(device);

    NSWindow* nswin = glfwGetCocoaWindow(window);
    NSView* view = nswin.contentView;
    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = device; layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.contentsScale = nswin.backingScaleFactor;
    layer.opaque = YES;

    [view setLayer:layer];
    [view setWantsLayer:YES]; // Korrekte Cocoa Reihenfolge

    std::vector<AxiomTrade> my_trades;
    static int next_id = 0, selected_id = -1;
    static std::string focus_target = "";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        CGSize bSize = view.bounds.size;
        CGFloat sc = layer.contentsScale;
        if (bSize.width <= 1) continue;
        layer.frame = view.bounds;
        layer.drawableSize = CGSizeMake(bSize.width * sc, bSize.height * sc);

        @autoreleasepool {
            id<CAMetalDrawable> drawable = [layer nextDrawable];
            if (!drawable) continue;

            // RenderPassDescriptor explizit konfigurieren
            MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];
            rp.colorAttachments[0].texture = drawable.texture;
            rp.colorAttachments[0].loadAction = MTLLoadActionClear;
            rp.colorAttachments[0].clearColor = MTLClearColorMake(0.01, 0.01, 0.015, 1.0);
            rp.colorAttachments[0].storeAction = MTLStoreActionStore;

            ImGui_ImplMetal_NewFrame(rp); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();

            // --- UI MASTER ---
            ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
            ImGui::Begin("SYNAPSE Master-Control");
            static char buf[64] = "BTC/USD"; static double e = 60000, ex = 61000;
            ImGui::InputText("Asset", buf, 64); ImGui::InputDouble("In", &e); ImGui::InputDouble("Out", &ex);
            
            if (selected_id == -1) {
                if (ImGui::Button("Add Trade", ImVec2(-1, 35)))
                    my_trades.push_back({next_id++, std::string(buf), e, ex, std::chrono::system_clock::now(), TradeStatus::REALIZED});
            } else {
                if (ImGui::Button("Update Selected", ImVec2(180, 35))) {
                    for(auto& t : my_trades) if(t.id == selected_id) { t.symbol = buf; t.entryPrice = e; t.exitPrice = ex; }
                }
                ImGui::SameLine(); if (ImGui::Button("Cancel", ImVec2(-1, 35))) selected_id = -1;
            }

            if (ImGui::BeginTable("T", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Asset"); ImGui::TableSetupColumn("PnL"); ImGui::TableSetupColumn("Edit");
                ImGui::TableHeadersRow();
                for (auto& t : my_trades) {
                    ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
                    if (ImGui::Selectable(t.symbol.c_str(), selected_id == t.id, ImGuiSelectableFlags_SpanAllColumns)) {
                        selected_id = t.id; strncpy(buf, t.symbol.c_str(), 64); e = t.entryPrice; ex = t.exitPrice;
                    }
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%.2f", t.exitPrice - t.entryPrice);
                    ImGui::TableSetColumnIndex(2); if(ImGui::SmallButton(("EDIT##"+std::to_string(t.id)).c_str())) focus_target = t.symbol;
                }
                ImGui::EndTable();
            }
            ImGui::End();

            // --- UI WORLD ---
            ImGui::SetNextWindowPos(ImVec2(20, 440), ImGuiCond_FirstUseEver);
            ImGui::Begin("World Monitor");
            RenderAxiomWorld(ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail(), my_trades, selected_id);
            ImGui::End();

            ImGui::Render();
            id<MTLCommandBuffer> cb = [commandQueue commandBuffer];
            // FIX: Explizite Adressierung des Encoders zur Behebung des Selector-Fehlers
            id<MTLRenderCommandEncoder> ce = [cb renderCommandEncoderWithDescriptor:rp];
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cb, ce);
            [ce endEncoding]; [cb presentDrawable:drawable]; [cb commit];
        }
    }
    return 0;
}

