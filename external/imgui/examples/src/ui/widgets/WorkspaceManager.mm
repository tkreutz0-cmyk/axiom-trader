// Patch v0.7.1: Single-Hub Time Zones
#pragma once
#include <imgui.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <charconv>
#include <string_view>

namespace workspace {

struct HubTZ { const char* name; int offset_minutes; };

static constexpr std::array<HubTZ, 5> kHubs = {
	HubTZ{"NYC", -4 * 60}, HubTZ{"LDN", 1 * 60},  HubTZ{"FRA", 2 * 60},
	HubTZ{"TYO", 9 * 60},  HubTZ{"SYD", 10 * 60}
};

// Helper: compute local hour/minute from epoch seconds and offset minutes
inline std::pair<int,int> ComputeLocalHourMinute(int64_t epoch_seconds, int offset_minutes) noexcept {
	int64_t secs = epoch_seconds;
	int64_t local = (secs % 86400) + static_cast<int64_t>(offset_minutes) * 60;
	local = (local + 86400) % 86400;
	int hour = static_cast<int>(local / 3600);
	int minute = static_cast<int>((local % 3600) / 60);
	return {hour, minute};
}

// Helper: compute text position relative to marker (deterministic)
inline ImVec2 ComputeHubTextPosition(ImVec2 markerPos) noexcept {
	return ImVec2(markerPos.x - 20.0f, markerPos.y + 12.0f);
}

inline void RenderSingleHubTimeZone(ImDrawList* dl, ImVec2 markerPos, std::string_view venue, ImU32 textCol = IM_COL32_WHITE) noexcept {
	if (!dl) return;
	const HubTZ* activeHub = nullptr;
	for (const auto& h : kHubs) { if (venue == h.name) { activeHub = &h; break; } }
	if (!activeHub) return;

	using namespace std::chrono;
	int64_t secs = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
	auto [hour, minute] = ComputeLocalHourMinute(secs, activeHub->offset_minutes);

	char buf[32]; char* p = buf;
	const char* name = activeHub->name; while (*name) { *p++ = *name++; } *p++ = ' ';

	if (hour < 10) { *p++ = '0'; *p++ = static_cast<char>('0' + hour); }
	else { char tmp[4]; auto res = std::to_chars(tmp, tmp+4, hour); for (char* k = tmp; k < res.ptr; ++k) *p++ = *k; }
	*p++ = ':';

	if (minute < 10) { *p++ = '0'; *p++ = static_cast<char>('0' + minute); }
	else { char tmp[4]; auto res = std::to_chars(tmp, tmp+4, minute); for (char* k = tmp; k < res.ptr; ++k) *p++ = *k; }

	ImVec2 pos = ComputeHubTextPosition(markerPos);
	dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), pos, textCol, buf, p);
}

} // namespace workspace
