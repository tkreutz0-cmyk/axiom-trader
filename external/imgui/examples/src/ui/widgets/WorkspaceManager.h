// Header for WorkspaceManager helpers
#pragma once

#include <imgui.h>
#include <utility>
#include <cstdint>
#include <string_view>

namespace workspace {

enum class TradingHub : uint8_t { NYC = 0, LDN = 1, FRA = 2, TYO = 3, SYD = 4, UNKNOWN = 255 };

struct HubTZ { TradingHub id; const char* iana_name; };

// Compute local hour/minute for a given epoch seconds and trading hub using std::chrono::zoned_time when available.
std::pair<int,int> ComputeLocalHourMinute(int64_t epoch_seconds, TradingHub hub) noexcept;
ImVec2 ComputeHubTextPosition(ImVec2 markerPos) noexcept;
void RenderSingleHubTimeZone(ImDrawList* dl, ImVec2 markerPos, TradingHub hub, ImU32 textCol = IM_COL32_WHITE) noexcept;

} // namespace workspace
