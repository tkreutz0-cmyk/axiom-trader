// WorldRenderer.h
#pragma once
#include <imgui.h>
#include <cstddef> // size_t

struct Trade
{
    float lat_deg; // [-90..+90]
    float lon_deg; // [-180..+180]
    bool is_buy;   // true = BUY, false = SELL
    // ... andere Felder (Preis/Menge/Zeit) sind fürs Zeichnen irrelevant
};

class WorldRenderer
{
public:
    // Wird innerhalb des "World Map" ImGui-Fensters aufgerufen.
    // trades: Bestehende Array- oder Vektordaten (Null-Allokation im Frame).
    void DrawWorldMapWithTrades(ImTextureID map_texture,
                                const ImVec2& map_size,
                                const Trade* trades,
                                const size_t trade_count) noexcept
    {
        // 1) Bildregion im Screen-Space bestimmen
        const ImVec2 map_min = ImGui::GetCursorScreenPos();
        const ImVec2 map_max = ImVec2(map_min.x + map_size.x, map_min.y + map_size.y);

        // 2) Weltkarten-Textur zeichnen
        ImGui::Image(map_texture, map_size);

        // 3) Trades über das Bild zeichnen
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->PushClipRect(map_min, map_max, true);

        // Radius skaliert sanft mit der Kartengröße, ist aber fest geklammert
        const float min_edge = (map_size.x < map_size.y) ? map_size.x : map_size.y;
        float radius = min_edge * 0.005f;
        if (radius < 3.0f) radius = 3.0f;
        if (radius > 8.0f) radius = 8.0f;

        // Farben vorab berechnen (Null-Allokation im Loop)
        const ImU32 col_buy = IM_COL32(60, 220, 80, 220);  // Grün-transparent
        const ImU32 col_sell = IM_COL32(220, 60, 60, 220); // Rot-transparent

        for (size_t i = 0; i < trade_count; ++i)
        {
            const Trade& t = trades[i];

            // Optionale Klammerung gegen NaNs oder korrupte Core-Daten
            float lat = t.lat_deg;
            float lon = t.lon_deg;
            if (lat > 90.0f)   lat = 90.0f;
            if (lat < -90.0f)  lat = -90.0f;
            if (lon > 180.0f)  lon = 180.0f;
            if (lon < -180.0f) lon = -180.0f;

            // Equirectangular-Transformation:
            // u in [0..1] aus lon [-180..180]
            // v in [0..1] aus lat [90..-90] (Top-Down Pixelkoordinaten)
            const float u = (lon + 180.0f) * (1.0f / 360.0f);
            const float v = (90.0f - lat) * (1.0f / 180.0f);

            const ImVec2 p = ImVec2(
                map_min.x + u * map_size.x,
                map_min.y + v * map_size.y
            );

            dl->AddCircleFilled(p, radius, t.is_buy ? col_buy : col_sell, 12);
        }
        
        dl->PopClipRect();
    }

    // Komfort-Überladung für std::vector oder andere Container ohne Allokations-Overhead
    template <class TradeVec>
    void DrawWorldMapWithTrades(ImTextureID map_texture,
                                const ImVec2& map_size,
                                const TradeVec& trades) noexcept
    {
        DrawWorldMapWithTrades(map_texture, map_size,
                               trades.data(), 
                               static_cast<size_t>(trades.size()));
    }
};
