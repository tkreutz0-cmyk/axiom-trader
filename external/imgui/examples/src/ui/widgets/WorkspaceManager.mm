// WorkspaceManager implementation
#include "WorkspaceManager.h"

#include <array>
#include <chrono>
#include <charconv>

namespace workspace {

struct HubTZ { TradingHub id; const char* iana_name; };

static constexpr std::array<HubTZ, 5> kHubs = {
	HubTZ{TradingHub::NYC, "America/New_York"},
	HubTZ{TradingHub::LDN, "Europe/London"},
	HubTZ{TradingHub::FRA, "Europe/Berlin"},
	HubTZ{TradingHub::TYO, "Asia/Tokyo"},
	HubTZ{TradingHub::SYD, "Australia/Sydney"}
};

// Compute local hour/minute for a given epoch seconds and trading hub. Uses std::chrono::zoned_time when available
// to correctly handle DST. Falls back to a conservative fixed-offset mapping if zoned_time is not available.
std::pair<int,int> ComputeLocalHourMinute(int64_t epoch_seconds, TradingHub hub) noexcept {
	using namespace std::chrono;

#if defined(__cpp_lib_chrono) && __has_include(<chrono>)
	try {
		const char* tzname = "UTC";
		for (const auto& h : kHubs) if (h.id == hub) { tzname = h.iana_name; break; }
		// Construct zoned_time using the IANA zone name and system_clock time_point
		system_clock::time_point tp{seconds(epoch_seconds)};
		auto tz = locate_zone(tzname);
		zoned_time zt{tz, tp};
		// get_local_time returns a local_time<system_clock::duration>
		local_time local = zt.get_local_time();
		// extract time of day
		auto days = floor<days>(local);
		auto tod = local - days;
		auto secs = floor<seconds>(tod);
		hh_mm_ss hms{secs};
		return {static_cast<int>(hms.hours().count()), static_cast<int>(hms.minutes().count())};
	} catch(...) {
		// fall through to fallback mapping
	}
#endif

	// Fallback conservative mapping (no DST handling). Kept only for platforms without zoned_time support.
	int offset_minutes = 0;
	switch (hub) {
		case TradingHub::NYC: offset_minutes = -4*60; break;
		case TradingHub::LDN: offset_minutes = 1*60; break;
		case TradingHub::FRA: offset_minutes = 2*60; break;
		case TradingHub::TYO: offset_minutes = 9*60; break;
		case TradingHub::SYD: offset_minutes = 10*60; break;
		default: offset_minutes = 0; break;
	}
	int64_t secs = epoch_seconds;
	int64_t local = (secs % 86400) + static_cast<int64_t>(offset_minutes) * 60;
	local = (local + 86400) % 86400;
	int hour = static_cast<int>(local / 3600);
	int minute = static_cast<int>((local % 3600) / 60);
	return {hour, minute};
}

ImVec2 ComputeHubTextPosition(ImVec2 markerPos) noexcept {
	return ImVec2(markerPos.x - 20.0f, markerPos.y + 12.0f);
}

void RenderSingleHubTimeZone(ImDrawList* dl, ImVec2 markerPos, TradingHub hub, ImU32 textCol) noexcept {
	if (!dl) return;
	const HubTZ* activeHub = nullptr;
	for (const auto& h : kHubs) { if (h.id == hub) { activeHub = &h; break; } }
	if (!activeHub) return;

	using namespace std::chrono;
	int64_t secs = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
	auto [hour, minute] = ComputeLocalHourMinute(secs, hub);

	char buf[32]; char* p = buf; char* end = buf + sizeof(buf);

	// write short hub name (e.g., "NYC") safely
	const char* shortName = "";
	switch (hub) {
		case TradingHub::NYC: shortName = "NYC"; break;
		case TradingHub::LDN: shortName = "LDN"; break;
		case TradingHub::FRA: shortName = "FRA"; break;
		case TradingHub::TYO: shortName = "TYO"; break;
		case TradingHub::SYD: shortName = "SYD"; break;
		default: shortName = "UNK"; break;
	}
	for (const char* q = shortName; *q; ++q) { if (p >= end) return; *p++ = *q; }
	if (p >= end) return; *p++ = ' ';

	// hour
	if (hour < 10) {
		if (p + 2 > end) return;
		*p++ = '0'; *p++ = static_cast<char>('0' + hour);
	} else {
		char tmp[4]; auto res = std::to_chars(tmp, tmp+4, hour);
		if (res.ec != std::errc()) return;
		size_t len = static_cast<size_t>(res.ptr - tmp);
		if (p + static_cast<ptrdiff_t>(len) > end) return;
		for (size_t k = 0; k < len; ++k) *p++ = tmp[k];
	}

	if (p >= end) return; *p++ = ':';

	// minute
	if (minute < 10) {
		if (p + 2 > end) return;
		*p++ = '0'; *p++ = static_cast<char>('0' + minute);
	} else {
		char tmp[4]; auto res = std::to_chars(tmp, tmp+4, minute);
		if (res.ec != std::errc()) return;
		size_t len = static_cast<size_t>(res.ptr - tmp);
		if (p + static_cast<ptrdiff_t>(len) > end) return;
		for (size_t k = 0; k < len; ++k) *p++ = tmp[k];
	}

	ImVec2 pos = ComputeHubTextPosition(markerPos);
	dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), pos, textCol, buf, p);
}

} // namespace workspace
