// WorkspaceManager.h
#pragma once
#include <array>
#include <imgui.h>

class WorkspaceManager
{
public:
    // Window IDs - Festes Layout ohne Laufzeit-Allokationen
    enum class WindowId : int
    {
        Chart = 0,
        OrderBook,
        TradesTape,
        Positions,
        WorldMap,
        Log,
        Count
    };

    // Öffentlicher Trigger für das UI-Kommando
    bool trigger_layout_reset = false;

    // Aufruf über das UI-Menü: "Reset Workspace"
    inline void RequestLayoutReset() noexcept { trigger_layout_reset = true; }

    // Muss einmal pro Frame ganz am Ende aufgerufen werden (nach allen Fenstern)
    inline void EndFrame() noexcept
    {
        trigger_layout_reset = false;
        layout_computed_this_frame_ = false;
    }

    // Sicherer Wrapper um ImGui::Begin, der das Layout beim Reset-Trigger erzwingt
    inline bool BeginWindow(WindowId id,
                            bool* p_open = nullptr,
                            ImGuiWindowFlags flags = 0) noexcept
    {
        if (trigger_layout_reset)
        {
            if (!layout_computed_this_frame_)
            {
                ComputeLayout_(ImGui::GetIO().DisplaySize);
                layout_computed_this_frame_ = true;
            }
            
            const LayoutRect& r = layout_[static_cast<int>(id)];
            ImGui::SetNextWindowPos(r.pos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(r.size, ImGuiCond_Always);
        }
        return ImGui::Begin(WindowName(id), p_open, flags);
    }

    inline void EndWindow() noexcept { ImGui::End(); }

    static inline const char* WindowName(WindowId id) noexcept
    {
        switch (id)
        {
            case WindowId::Chart:      return "Chart";
            case WindowId::OrderBook:  return "Order Book";
            case WindowId::TradesTape: return "Trades";
            case WindowId::Positions:  return "Positions";
            case WindowId::WorldMap:   return "World Map";
            case WindowId::Log:        return "Log";
            default:                   return "Window";
        }
    }

private:
    struct LayoutRect
    {
        ImVec2 pos;
        ImVec2 size;
    };

    std::array<LayoutRect, static_cast<int>(WindowId::Count)> layout_{};
    bool layout_computed_this_frame_ = false;

    static inline float ClampMin_(float v, float minv) noexcept
    {
        return (v < minv) ? minv : v;
    }

    inline void ComputeLayout_(const ImVec2 display) noexcept
    {
        const float w = display.x;
        const float h = display.y;

        // Symmetrische Verhältnisse: 25% | 50% | 25% Spalten; 65% obere / 35% untere Zeile
        const float leftW   = w * 0.25f;
        const float centerW = w * 0.50f;
        const float rightW  = w * 0.25f;
        const float topH    = h * 0.65f;
        const float botH    = h * 0.35f;

        // Mindestmaße um unbrauchbar winzige Boxen bei Mini-Auflösungen abzufangen
        const float minW = 320.0f;
        const float minH = 180.0f;

        // Linke Spalte
        layout_[static_cast<int>(WindowId::OrderBook)] = {
            ImVec2(0.0f, 0.0f),
            ImVec2(ClampMin_(leftW, minW), ClampMin_(topH, minH))
        };
        layout_[static_cast<int>(WindowId::TradesTape)] = {
            ImVec2(0.0f, topH),
            ImVec2(ClampMin_(leftW, minW), ClampMin_(botH, minH))
        };

        // Mittlere Spalte
        layout_[static_cast<int>(WindowId::Chart)] = {
            ImVec2(leftW, 0.0f),
            ImVec2(ClampMin_(centerW, minW), ClampMin_(topH, minH))
        };
        layout_[static_cast<int>(WindowId::Log)] = {
            ImVec2(leftW, topH),
            ImVec2(ClampMin_(centerW, minW), ClampMin_(botH, minH))
        };

        // Rechte Spalte
        layout_[static_cast<int>(WindowId::Positions)] = {
            ImVec2(leftW + centerW, 0.0f),
            ImVec2(ClampMin_(rightW, minW), ClampMin_(topH, minH))
        };
        layout_[static_cast<int>(WindowId::WorldMap)] = {
            ImVec2(leftW + centerW, topH),
            ImVec2(ClampMin_(rightW, minW), ClampMin_(botH, minH))
        };
    }
};

