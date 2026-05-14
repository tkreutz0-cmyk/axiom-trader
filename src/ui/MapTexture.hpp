#pragma once

#include <string>
#include <cstdint>
#include <imgui.h>

// MapTexture kapselt:
//  - Laden eines Bildes via stb_image (CPU-Raw-Bytes)
//  - Erstellen/Verwalten einer Metal-Textur (GPU-Resource)
//  - Casting zu ImTextureID (ImGui Metal Backend: void* auf id<MTLTexture>)
//
// Hinweis: Der Header ist absichtlich "reines C++".
// Metal-Objekte werden als void* abstrahiert und in der .mm implementiert.

class MapTexture final
{
public:
    MapTexture() noexcept;
    explicit MapTexture(const std::string& filePath, bool flipVertical = false);
    ~MapTexture() noexcept;

    MapTexture(const MapTexture&) = delete;
    MapTexture& operator=(const MapTexture&) = delete;

    MapTexture(MapTexture&& other) noexcept;
    MapTexture& operator=(MapTexture&& other) noexcept;

    // 1) Speicher-Kapselung (Core): Laden via stb_image
    // Lädt das Bild als RGBA8 in CPU-Speicher (stbi_image_free im Destruktor).
    // Rückgabe: true bei Erfolg.
    bool loadFromFile(const std::string& filePath, bool flipVertical = false) noexcept;

    // 2) Metal-Textur-Initialisierung (Render-Schicht):
    // Erwartet das aktuelle MTLDevice als void* (tatsächlich: id<MTLDevice>).
    // Erzeugt eine id<MTLTexture> (RGBA8Unorm) und kopiert die Pixeldaten via replaceRegion.
    // Rückgabe: true bei Erfolg.
    bool uploadToMetal(void* mtlDevice) noexcept;

    // Freigabe von CPU-Bytes und GPU-Texture (RAII Reset)
    void reset() noexcept;

    // Zugriff
    int width() const noexcept;
    int height() const noexcept;
    int channels() const noexcept;          // wird bei dieser Implementierung 4 sein (RGBA)
    const std::uint8_t* pixels() const noexcept;

    // GPU-Handle (tatsächlich: id<MTLTexture>, aber als void* abstrahiert)
    void* metalTexture() const noexcept;

    // 3) ImGui-Rendering Pipeline:
    // Metal-Backend nutzt ImTextureID als void* auf id<MTLTexture>.
    // Diese Funktion liefert das korrekte ImTextureID.
    ImTextureID imguiTextureID() const noexcept;

private:
    struct Impl;
    Impl* impl_ = nullptr;
};

// Optionaler Helper, wenn du die Integration sauber kapseln willst:
// (Kann direkt in deinem UI-Render-Loop aufgerufen werden.)
void DrawTradingMapWindow(MapTexture& mapTexture, float scale = 1.0f) noexcept;
