// axiom_trader.mm
#include <stdio.h>
#include <string>
#include <filesystem>
#include <vector>
#include <cmath> // Für sin/cos

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

namespace fs = std::filesystem;

/* =========================
   macOS Pfad-Helfer
   ========================= */
static NSURL* AXIOMBaseURL_AppSupport() {
    NSFileManager* fm = [NSFileManager defaultManager];
    NSURL* appSupport = [fm URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask].firstObject;
    NSURL* axiom = [appSupport URLByAppendingPathComponent:@"AXIOM" isDirectory:YES];
    NSError* err = nil;
    if (![fm createDirectoryAtURL:axiom withIntermediateDirectories:YES attributes:nil error:&err]) return nil;
    return axiom;
}

static std::string URLToUTF8Path(NSURL* url) {
    return (url) ? std::string(url.path.UTF8String) : "";
}

/* =========================
   Projekt-Speicherlogik
   ========================= */
static bool SaveProjectAnchor_NS(const std::string& folder, std::string& outErr, std::string& outPath) {
    @autoreleasepool {
        NSFileManager* fm = [NSFileManager defaultManager];
        NSURL* projURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:folder.c_str()] isDirectory:YES];
        NSError* err = nil;
        if (![fm createDirectoryAtURL:[projURL URLByAppendingPathComponent:@"src"] withIntermediateDirectories:YES attributes:nil error:&err]) {
            outErr = err.localizedDescription.UTF8String; return false;
        }
        NSString* content = @"AXIOM-TRADER-V0.1\nSTATUS: INITIALIZED\n";
        NSURL* fileURL = [projURL URLByAppendingPathComponent:@"project.axiom"];
        if (![content writeToURL:fileURL atomically:YES encoding:NSUTF8StringEncoding error:&err]) {
            outErr = err.localizedDescription.UTF8String; return false;
        }
        outPath = URLToUTF8Path(fileURL);
        return true;
    }
}

int main(int, char**) {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "AXIOM Trader v0.1", nullptr, nullptr);
    
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
    nswin.contentView.wantsLayer = YES;
    nswin.contentView.layer = layer;

    MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];
    std::string baseAbs = URLToUTF8Path(AXIOMBaseURL_AppSupport());
    static char projectPath[512];
    snprintf(projectPath, sizeof(projectPath), "%s/my_trading_project", baseAbs.c_str());
    std::string lastStatus = "Bereit.";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        @autoreleasepool {
            CGSize size = nswin.contentView.bounds.size;
            layer.drawableSize = CGSizeMake(size.width * nswin.backingScaleFactor, size.height * nswin.backingScaleFactor);
            id<CAMetalDrawable> drawable = [layer nextDrawable];
            if (!drawable) continue;

            rp.colorAttachments[0].texture = drawable.texture;
            rp.colorAttachments[0].loadAction = MTLLoadActionClear;
            rp.colorAttachments[0].clearColor = MTLClearColorMake(0.08, 0.08, 0.1, 1.0);

            ImGui_ImplMetal_NewFrame(rp);
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 1. Fenster: Control
            ImGui::Begin("AXIOM Control");
            ImGui::InputText("Pfad", projectPath, 512);
            if (ImGui::Button("Speichern")) {
                std::string err, path;
                if (SaveProjectAnchor_NS(projectPath, err, path)) lastStatus = "OK: " + path;
                else lastStatus = "Fehler: " + err;
            }
            ImGui::Text("%s", lastStatus.c_str());
            ImGui::End();

            // 2. Fenster: DEIN NEUER DYNAMISCHER BLOCK
            ImGui::Begin("AXIOM Needle-Dashboard Prototype");
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 origin = ImGui::GetCursorScreenPos();
                double time = ImGui::GetTime();

                for (int i = 0; i < 7; i++) {
                    float x_pos = origin.x + 50.0f + (i * 70.0f);
                    float y_base = origin.y + 150.0f;
                    
                    float wave = (float)sin(time * 1.5f + i);
                    float noise = 20.0f + (wave * 10.0f);
                    float pnl = 30.0f + (float)cos(time * 0.8f + i) * 15.0f;

                    bool is_hovered = ImGui::IsMouseHoveringRect(ImVec2(x_pos - 15, y_base - 40),
                                                                 ImVec2(x_pos + 15, y_base + 60));
                    
                    ImU32 col_needle = is_hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(150, 150, 150, 200);
                    ImU32 col_body   = (i % 2 == 0) ? IM_COL32(46, 204, 113, 200) : IM_COL32(231, 76, 60, 200);

                    draw_list->AddLine(ImVec2(x_pos, y_base - noise),
                                       ImVec2(x_pos, y_base + pnl + noise), col_needle, 1.0f);
                    draw_list->AddRectFilled(ImVec2(x_pos - 10, y_base),
                                             ImVec2(x_pos + 10, y_base + pnl), col_body, 3.0f);
                    draw_list->AddCircleFilled(ImVec2(x_pos, y_base), 3.0f, IM_COL32(255, 255, 255, 255));
                }
                ImGui::Dummy(ImVec2(550, 300));
            }
            ImGui::End();

            // Rendering
            ImGui::Render();
            id<MTLCommandBuffer> cb = [commandQueue commandBuffer];
            id<MTLRenderCommandEncoder> ence = [cb renderCommandEncoderWithDescriptor:rp];
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cb, ence);
            [ence endEncoding];
            [cb presentDrawable:drawable];
            [cb commit];
        }
    }
    return 0;
}

