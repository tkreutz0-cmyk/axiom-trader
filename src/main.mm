// axiom_trader.mm
#include <stdio.h>
#include <string>
#include <vector>
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

enum class TradeStatus { PLANNED = 0, REALIZED = 1 };

struct AxiomTrade {
    std::string symbol;
    double entryPrice;
    double exitPrice;
    TradeStatus status;
    ImU32 colEntry = IM_COL32(0, 122, 255, 255);
    ImU32 colExit = IM_COL32(255, 59, 48, 255);
    ImU32 colPlanned = IM_COL32(255, 204, 0, 255);
};

int main(int, char**) {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    
    GLFWwindow* window = glfwCreateWindow(1440, 900, "AXIOM Trader v0.1", nullptr, nullptr);
    if (!window) return 1;

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    id<MTLCommandQueue> commandQueue = [device newCommandQueue];

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    // Initialisierung mit modernem Callback-Helper
    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplGlfw_InstallCallbacks(window);
    ImGui_ImplMetal_Init(device);

    NSWindow* nswin = glfwGetCocoaWindow(window);
    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.wantsLayer = YES;
    nswin.contentView.layer = layer;

    // Aktuell flüchtiger Speicher (Vektor)
    std::vector<AxiomTrade> my_trades = {
        {"BTC/USD", 60000.0, 64000.0, TradeStatus::REALIZED},
        {"ETH/USD", 3200.0, 3100.0, TradeStatus::REALIZED}
    };

    static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
    static float zoom = 1.0f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        layer.drawableSize = CGSizeMake(width, height);

        @autoreleasepool {
            MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];
            id<CAMetalDrawable> drawable = [layer nextDrawable];
            if (!drawable) continue;

            rp.colorAttachments[0].texture = drawable.texture;
            rp.colorAttachments[0].loadAction = MTLLoadActionClear;
            rp.colorAttachments[0].clearColor = MTLClearColorMake(0.01, 0.01, 0.02, 1.0);

            ImGui_ImplMetal_NewFrame(rp);
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 1. SYNAPSE-JOURNAL (Eingabe)
            ImGui::SetNextWindowPos(ImVec2(20, 50), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(320, 350), ImGuiCond_FirstUseEver);
            ImGui::Begin("SYNAPSE-Journal");
            {
                static char buf_symbol[64] = "BTC/USD";
                static double val_entry = 60000.0;
                static double val_exit = 61000.0;
                static int status_idx = 1;
                const char* status_names[] = { "Geplant", "Realisiert" };

                ImGui::InputText("Instrument", buf_symbol, 64);
                ImGui::InputDouble("Einstieg", &val_entry);
                if (status_idx == 1) ImGui::InputDouble("Ausstieg", &val_exit);
                ImGui::Combo("Status", &status_idx, status_names, 2);

                if (ImGui::Button("Trade hinzufügen", ImVec2(-1, 40))) {
                    AxiomTrade nt;
                    nt.symbol = buf_symbol;
                    nt.entryPrice = val_entry;
                    nt.exitPrice = (status_idx == 1) ? val_exit : 0.0;
                    nt.status = (status_idx == 1) ? TradeStatus::REALIZED : TradeStatus::PLANNED;
                    my_trades.push_back(nt);
                }
            }
            ImGui::End();

            // 2. AXIOM DASHBOARD (Visualisierung)
            ImGui::SetNextWindowPos(ImVec2(360, 50), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(1000, 800), ImGuiCond_FirstUseEver);
            ImGui::Begin("AXIOM Dashboard");
            {
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ImVec2 sz = ImGui::GetContentRegionAvail();
                ImVec2 p1 = ImVec2(p0.x + sz.x, p0.y + sz.y);

                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                draw_list->AddRectFilled(p0, p1, IM_COL32(10, 10, 15, 255));

                ImGui::InvisibleButton("canvas_btn", sz);
                if (ImGui::IsItemHovered()) {
                    if (ImGui::GetIO().KeyAlt) {
                        zoom += ImGui::GetIO().MouseWheel * 0.05f * zoom;
                        if (zoom < 0.01f) zoom = 0.01f;
                    } else {
                        scrolling.x += ImGui::GetIO().MouseWheelH * 60.0f;
                        scrolling.y += ImGui::GetIO().MouseWheel * 60.0f;
                    }
                }

                ImVec2 origin = ImVec2(p0.x + scrolling.x, p0.y + scrolling.y);
                draw_list->PushClipRect(p0, p1, true);
                
                for (size_t n = 0; n < my_trades.size(); n++) {
                    const auto& t = my_trades[n];
                    float x = origin.x + (n * 150.0f * zoom);
                    float y_mid = origin.y + (sz.y / 2.0f);

                    if (t.status == TradeStatus::REALIZED) {
                        float y_e = y_mid - (float)(t.exitPrice - t.entryPrice) * 0.05f * zoom;
                        draw_list->AddLine(ImVec2(x, y_mid), ImVec2(x, y_e), IM_COL32(180, 180, 180, 255), 2.0f);
                        draw_list->AddCircleFilled(ImVec2(x, y_mid), 6.0f * zoom, t.colEntry);
                        draw_list->AddCircleFilled(ImVec2(x, y_e), 6.0f * zoom, t.colExit);
                    } else {
                        draw_list->AddTriangleFilled(ImVec2(x, y_mid-8*zoom), ImVec2(x-8*zoom, y_mid+8*zoom), ImVec2(x+8*zoom, y_mid+8*zoom), t.colPlanned);
                    }
                    draw_list->AddText(NULL, 15.0f * zoom, ImVec2(x + 10, y_mid), IM_COL32_WHITE, t.symbol.c_str());
                }
                draw_list->PopClipRect();
            }
            ImGui::End();

            // Rendering
            ImGui::Render();
            id<MTLCommandBuffer> cb = [commandQueue commandBuffer];
            id<MTLRenderCommandEncoder> enc = [cb renderCommandEncoderWithDescriptor:rp];
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cb, enc);
            [enc endEncoding];
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

