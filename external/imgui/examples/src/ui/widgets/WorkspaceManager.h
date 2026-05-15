// Header for WorkspaceManager helpers
#pragma once

#include <imgui.h>
#include <utility>
#include <cstdint>
#include <string_view>

namespace workspace {

struct HubTZ;

std::pair<int,int> ComputeLocalHourMinute(int64_t epoch_seconds, int offset_minutes) noexcept;
ImVec2 ComputeHubTextPosition(ImVec2 markerPos) noexcept;
void RenderSingleHubTimeZone(ImDrawList* dl, ImVec2 markerPos, std::string_view venue, ImU32 textCol = IM_COL32_WHITE) noexcept;

} // namespace workspace
