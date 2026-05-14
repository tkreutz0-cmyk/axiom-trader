#import <TargetConditionals.h>
#import <Metal/Metal.h>

#include "MapTexture.hpp"

#include <algorithm>
#include <new>


// stb_image: Laden von Bilddateien (PNG/JPG/etc.)
//
// WICHTIG:
// In vielen Projekten wird stb_image bereits in genau EINER Translation Unit mit
// STB_IMAGE_IMPLEMENTATION kompiliert. Um Duplicate-Symbols zu vermeiden,
// kannst du extern kompilieren und vor diesem Include AXIOM_TRADER_EXTERNAL_STB_IMAGE_IMPL definieren.
//
// Wenn du *keine* zentrale stb_image.cpp/.mm hast, lässt du den Default so:
// -> Dann wird die Implementation hier mit kompiliert.
#ifndef AXIOM_TRADER_EXTERNAL_STB_IMAGE_IMPL
    #ifndef STB_IMAGE_IMPLEMENTATION
        #define STB_IMAGE_IMPLEMENTATION
    #endif
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace
{
    static inline id<MTLDevice> ToMTLDevice(void* p) noexcept
    {
    #if __has_feature(objc_arc)
        return (__bridge id<MTLDevice>)p;
    #else
        return (id<MTLDevice>)p;
    #endif
    }

    static inline void* ToVoidPtr(id obj) noexcept
    {
    #if __has_feature(objc_arc)
        return (__bridge void*)obj;
    #else
        return (void*)obj;
    #endif
    }
}

struct MapTexture::Impl
{
    stbi_uc* pixels = nullptr;
    int w = 0;
    int h = 0;
    int c = 0; // tatsächliche Kanäle im Speicher (wir forcen RGBA = 4)

    id<MTLTexture> texture = nil;

    void clearCPU() noexcept
    {
        if (pixels)
        {
            stbi_image_free(pixels);
            pixels = nullptr;
        }
        w = h = c = 0;
    }

    void clearGPU() noexcept
    {
        if (texture != nil)
        {
        #if !__has_feature(objc_arc)
            [texture release];
        #endif
            texture = nil;
        }
    }

    void reset() noexcept
    {
        clearGPU();
        clearCPU();
    }

    ~Impl() noexcept
    {
        reset();
    }
};

MapTexture::MapTexture() noexcept
{
    impl_ = new (std::nothrow) Impl();
}

MapTexture::MapTexture(const std::string& filePath, bool flipVertical)
    : MapTexture()
{
    (void)loadFromFile(filePath, flipVertical);
}

MapTexture::~MapTexture() noexcept
{
    delete impl_;
    impl_ = nullptr;
}

MapTexture::MapTexture(MapTexture&& other) noexcept
{
    impl_ = other.impl_;
    other.impl_ = nullptr;
}

MapTexture& MapTexture::operator=(MapTexture&& other) noexcept
{
    if (this != &other)
    {
        delete impl_;
        impl_ = other.impl_;
        other.impl_ = nullptr;
    }
    return *this;
}

bool MapTexture::loadFromFile(const std::string& filePath, bool flipVertical) noexcept
{
    if (!impl_)
        impl_ = new (std::nothrow) Impl();

    if (!impl_)
        return false;

    // alte Ressourcen frei
    impl_->clearCPU();

    stbi_set_flip_vertically_on_load(flipVertical ? 1 : 0);

    int w = 0, h = 0, ch = 0;

    // Forciere RGBA (4 Kanäle), wie für MTLPixelFormatRGBA8Unorm erwartet.
    stbi_uc* data = stbi_load(filePath.c_str(), &w, &h, &ch, 4);
    if (!data || w <= 0 || h <= 0)
    {
        if (data) stbi_image_free(data);
        return false;
    }

    impl_->pixels = data;
    impl_->w = w;
    impl_->h = h;
    impl_->c = 4;

    return true;
}

bool MapTexture::uploadToMetal(void* mtlDevice) noexcept
{
    if (!impl_ || !impl_->pixels || impl_->w <= 0 || impl_->h <= 0)
        return false;

    id<MTLDevice> device = ToMTLDevice(mtlDevice);
    if (!device)
        return false;

    // alte GPU Resource frei (wir behalten CPU Bytes; damit kann man re-uploaden)
    impl_->clearGPU();

    const NSUInteger width  = static_cast<NSUInteger>(impl_->w);
    const NSUInteger height = static_cast<NSUInteger>(impl_->h);

    // Setup gemäß Vorgabe:
    // - 2D
    // - PixelFormatRGBA8Unorm
    // - Breite/Höhe aus stb_image
    MTLTextureDescriptor* desc =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                           width:width
                                                          height:height
                                                       mipmapped:NO];

    // Minimales sinnvolles Usage-Flag für UI-Rendering (ShaderRead)
    desc.usage = MTLTextureUsageShaderRead;

    // StorageMode: Shared vermeidet Synchronisations-Fallen bei replaceRegion,
    // und ist für UI-Texturen zuverlässig (macOS/iOS).
    desc.storageMode = MTLStorageModeShared;

    id<MTLTexture> tex = [device newTextureWithDescriptor:desc];
    if (!tex)
        return false;

    const NSUInteger bytesPerPixel = 4;
    const NSUInteger bytesPerRow = width * bytesPerPixel;

    const MTLRegion region = MTLRegionMake2D(0, 0, width, height);

    // Pixeldaten kopieren (CPU → GPU)
    [tex replaceRegion:region
          mipmapLevel:0
            withBytes:impl_->pixels
          bytesPerRow:bytesPerRow];

    tex.label = @"axiom-trader.worldmap";

#if __has_feature(objc_arc)
    impl_->texture = tex; // ARC hält strong reference
#else
    impl_->texture = [tex retain];
    [tex release];
#endif

    return true;
}

void MapTexture::reset() noexcept
{
    if (impl_)
        impl_->reset();
}

int MapTexture::width() const noexcept
{
    return impl_ ? impl_->w : 0;
}

int MapTexture::height() const noexcept
{
    return impl_ ? impl_->h : 0;
}

int MapTexture::channels() const noexcept
{
    return impl_ ? impl_->c : 0;
}

const std::uint8_t* MapTexture::pixels() const noexcept
{
    return impl_ ? reinterpret_cast<const std::uint8_t*>(impl_->pixels) : nullptr;
}

void* MapTexture::metalTexture() const noexcept
{
    if (!impl_ || impl_->texture == nil)
        return nullptr;

    return ToVoidPtr(impl_->texture);
}

ImTextureID MapTexture::imguiTextureID() const noexcept
{
    // ImGui Metal backend expects ImTextureID to be a void* pointing to id<MTLTexture>.
    // Wir geben daher genau dieses "void*" zurück.
    return reinterpret_cast<ImTextureID>(metalTexture());
}

// 3) ImGui-Rendering Pipeline Integration
void DrawTradingMapWindow(MapTexture& mapTexture, float scale) noexcept
{
    // Fenster-Scoping absichern
    ImGui::Begin("Trading Map");
    ImGui::PushID("axiom-trader.trading-map");

    if (mapTexture.metalTexture() != nullptr)
    {
        const float s = std::max(0.01f, scale);
        const ImVec2 size{
            static_cast<float>(mapTexture.width())  * s,
            static_cast<float>(mapTexture.height()) * s
        };

        // Sicheres Casting (Metal backend: void* auf id<MTLTexture>)
        ImTextureID texId = mapTexture.imguiTextureID();

        ImGui::Image(texId, size);
    }
    else
    {
        ImGui::TextUnformatted("Weltkarte nicht geladen oder nicht zu Metal hochgeladen.");
    }

    ImGui::PopID();
    ImGui::End();
}
