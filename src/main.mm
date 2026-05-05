// axiom_trader.mm
#include <stdio.h>
#include <string>
#include <filesystem>

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
   macOS paths (robust)
   ========================= */
static NSURL* AXIOMBaseURL_AppSupport()
{
    NSFileManager* fm = [NSFileManager defaultManager];

    // ~/Library/Application Support
    NSURL* appSupport =
        [fm URLsForDirectory:NSApplicationSupportDirectory
                   inDomains:NSUserDomainMask].firstObject;

    // ~/Library/Application Support/AXIOM
    NSURL* axiom = [appSupport URLByAppendingPathComponent:@"AXIOM" isDirectory:YES];

    NSError* err = nil;
    BOOL ok = [fm createDirectoryAtURL:axiom
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:&err];

    if (!ok) {
        fprintf(stderr, "ERROR: createDirectory(AppSupport/AXIOM) failed: %s\n",
                err ? err.localizedDescription.UTF8String : "unknown");
        return nil;
    }
    return axiom;
}

static std::string URLToUTF8Path(NSURL* url)
{
    if (!url) return {};
    return std::string(url.path.UTF8String);
}

/* =========================
   Save Anchor (Black Box)
   ========================= */
static bool SaveProjectAnchor_NS(const std::string& projectFolderAbs, std::string& outError, std::string& outSavedFileAbs)
{
    @autoreleasepool {
        NSFileManager* fm = [NSFileManager defaultManager];

        NSURL* projURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:projectFolderAbs.c_str()]
                                    isDirectory:YES];

        // Create folders: src, docs
        NSError* err = nil;
        BOOL ok1 = [fm createDirectoryAtURL:[projURL URLByAppendingPathComponent:@"src" isDirectory:YES]
                withIntermediateDirectories:YES attributes:nil error:&err];
        if (!ok1) {
            outError = std::string("createDirectory src failed: ") + (err ? err.localizedDescription.UTF8String : "unknown");
            return false;
        }

        err = nil;
        BOOL ok2 = [fm createDirectoryAtURL:[projURL URLByAppendingPathComponent:@"docs" isDirectory:YES]
                withIntermediateDirectories:YES attributes:nil error:&err];
        if (!ok2) {
            outError = std::string("createDirectory docs failed: ") + (err ? err.localizedDescription.UTF8String : "unknown");
            return false;
        }

        // Write file
        NSString* content = @"AXIOM-TRADER-V0.1\nSTATUS: INITIALIZED\n";
        NSURL* fileURL = [projURL URLByAppendingPathComponent:@"project.axiom" isDirectory:NO];

        err = nil;
        BOOL wrote = [content writeToURL:fileURL atomically:YES encoding:NSUTF8StringEncoding error:&err];
        if (!wrote) {
            outError = std::string("writeToURL failed: ") + (err ? err.localizedDescription.UTF8String : "unknown");
            return false;
        }

        outSavedFileAbs = URLToUTF8Path(fileURL);
        return true;
    }
}

/* =========================
   Main
   ========================= */
int main(int, char**)
{
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "AXIOM Trader v0.1", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) { fprintf(stderr, "No Metal device.\n"); return 1; }
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
    layer.framebufferOnly = YES;
    nswin.contentView.wantsLayer = YES;
    nswin.contentView.layer = layer;

    MTLRenderPassDescriptor* rp = [MTLRenderPassDescriptor renderPassDescriptor];

    // Default path: ~/Library/Application Support/AXIOM/my_trading_project
    NSURL* baseURL = AXIOMBaseURL_AppSupport();
    std::string baseAbs = URLToUTF8Path(baseURL);
    std::string defaultProjectAbs = baseAbs.empty() ? std::string() : (baseAbs + "/my_trading_project");

    static char projectPath[512];
    snprintf(projectPath, sizeof(projectPath), "%s", defaultProjectAbs.c_str());

    std::string lastStatus = "Ready.";
    std::string lastSavedFile = "";

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        @autoreleasepool
        {
            CGSize size = nswin.contentView.bounds.size;
            CGFloat scale = nswin.backingScaleFactor;
            layer.drawableSize = CGSizeMake(size.width * scale, size.height * scale);

            id<CAMetalDrawable> drawable = [layer nextDrawable];
            if (!drawable) continue;

            rp.colorAttachments[0].texture = drawable.texture;
            rp.colorAttachments[0].loadAction = MTLLoadActionClear;
            rp.colorAttachments[0].storeAction = MTLStoreActionStore;
            rp.colorAttachments[0].clearColor = MTLClearColorMake(0.10, 0.10, 0.10, 1.0);

            ImGui_ImplMetal_NewFrame(rp);
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("AXIOM Control");
            ImGui::Text("Default Base (AppSupport):");
            ImGui::TextWrapped("%s", baseAbs.c_str());

            ImGui::Separator();
            ImGui::InputText("Projektpfad (absolut empfohlen)", projectPath, IM_ARRAYSIZE(projectPath));

            if (ImGui::Button("Projektanker speichern"))
            {
                std::string err, saved;
                // Wenn User relativ eingibt: relativ -> an Base anhaengen (Finder-Start CWD ist unzuverlässig)
                fs::path p(projectPath);
                std::string targetAbs;

                if (p.is_absolute() || baseAbs.empty()) {
                    targetAbs = p.string();
                } else {
                    targetAbs = (fs::path(baseAbs) / p).string();
                }

                bool ok = SaveProjectAnchor_NS(targetAbs, err, saved);
                if (ok) {
                    lastStatus = "Saved OK.";
                    lastSavedFile = saved;
                    printf("SAVED: %s\n", saved.c_str());
                } else {
                    lastStatus = "Save FAILED: " + err;
                    lastSavedFile.clear();
                    fprintf(stderr, "%s\n", lastStatus.c_str());
                }
            }

            ImGui::Separator();
            ImGui::TextWrapped("Status: %s", lastStatus.c_str());
            if (!lastSavedFile.empty()) {
                ImGui::TextWrapped("File: %s", lastSavedFile.c_str());
            }
            ImGui::End();

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

