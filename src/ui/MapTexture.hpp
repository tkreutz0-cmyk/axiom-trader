#pragma once
#include <string>
#include <cstdint>

// MapTexture kapselt:
// - Laden eines Bildes via stb_image (CPU-Raw-Bytes)
// - Erstellen/Verwalten einer Metal-Textur (GPU-Resource)
// - Bereitstellung eines plattformunabhängigen Texture-Handles
//
// Hinweis: Der Header ist absichtlich "reines C++" ohne ImGui-Abhängigkeiten.
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
 int channels() const noexcept; // wird bei dieser Implementierung 4 sein (RGBA)
 const std::uint8_t* pixels() const noexcept;

 // GPU-Handle (tatsächlich: id<MTLTexture>, aber als void* abstrahiert)
 void* metalTexture() const noexcept;

 // 3) ImGui-Rendering Pipeline Integration:
 // Liefert den rohen Texturzeiger, der im UI-Layer gecastet werden kann.
 void* textureHandle() const noexcept;

private:
 struct Impl;
 Impl* impl_ = nullptr;
};

// Optionaler Helper für den UI-Layer
void DrawTradingMapWindow(MapTexture& mapTexture, float scale = 1.0f) noexcept;

