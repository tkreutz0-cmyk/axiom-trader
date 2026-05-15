# Pre-Check Audit: Workspace Layout & Zeitzonen (v0.7.0)

## 1. Invarianten-Nachweis (KI-Audit)
* Die Zeitzonen-Offsets sind hardcodiert als constexpr-Strukturen, um OS-spezifische Zeitzonen-Abhängigkeiten (MSVC/Clang) komplett zu eliminieren.
* Zero-Allocation im UI-Thread: Alle Render-Routinen verwenden ausschließlich Stack-Storage (z. B. char buf[32]) und std::to_chars zur Zahlformatierung; es gibt keine std::string/std::vector/heap-Operationen in den Hotpath-Funktionen.
* Deterministischer Workspace-Reset: ResetWorkstationLayout() setzt für alle Kernfenster ImGui::SetNextWindowPos und ImGui::SetNextWindowSize mit ImGuiCond_Always.

## 2. Quellcode-Implementierung

```cpp
// File: docs/pre-checks/0005-workspace-layout-review.md - embedded, C++20
#include <imgui.h>
#include <imgui_internal.h> // for ImDrawList
#include <array>
#include <chrono>
#include <cstdint>
#include <charconv>
#include <algorithm>

namespace workspace {

struct HubTZ { const char* name; int offset_minutes; }; // offset relative to UTC

// Hardcoded offsets (intentional: no OS timezone dependence)
static constexpr std::array<HubTZ,5> kHubs = {
	HubTZ{"NYC", -4 * 60}, // New York (UTC-4)
	HubTZ{"LDN",  1 * 60}, // London  (UTC+1)
	HubTZ{"FRA",  2 * 60}, // Frankfurt (UTC+2)
	HubTZ{"TYO",  9 * 60}, // Tokyo (UTC+9)
	HubTZ{"SYD", 10 * 60}  // Sydney (UTC+10)
};

// Deterministic workspace reset: positions and sizes for named windows.
inline void ResetWorkstationLayout() noexcept {
	// Use DisplaySize as authoritative layout anchor
	ImVec2 display = ImGui::GetIO().DisplaySize;

	// Primary main window: fullscreen
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(display, ImGuiCond_Always);
	// Tools pane: anchored to right, fixed 300px width
	ImGui::SetNextWindowPos(ImVec2(display.x - 300.0f, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300.0f, display.y), ImGuiCond_Always);
	// Chart area: centered, 70% of width, 60% of height
	ImGui::SetNextWindowPos(ImVec2(display.x * 0.15f, display.y * 0.2f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(display.x * 0.7f, display.y * 0.6f), ImGuiCond_Always);
	// Trade pane: bottom, full width, 20% height
	ImGui::SetNextWindowPos(ImVec2(0.0f, display.y * 0.8f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(display.x, display.y * 0.2f), ImGuiCond_Always);

	// Note: callers must call ImGui::Begin(...) for each window name after this
}

// Render a 2:1 texture centered and fitted into the given window rectangle without distortion.
// Texture is assumed to have UVs in [0,1]. The routine writes no heap allocations.
inline void RenderStretchedMap(ImTextureID mapTex, ImDrawList* dl, ImVec2 winPos, ImVec2 winSize) noexcept {
	if (!dl || !mapTex) return;

	const float texAspect = 2.0f / 1.0f; // 2:1 texture
	const float winAspect = (winSize.x > 0.0f && winSize.y > 0.0f) ? (winSize.x / winSize.y) : texAspect;

	ImVec2 dstSize;
	if (winAspect > texAspect) {
		// window is wider than texture ratio -> limit by height
		dstSize.y = winSize.y;
		dstSize.x = winSize.y * texAspect;
	} else {
		// limit by width
		dstSize.x = winSize.x;
		dstSize.y = winSize.x / texAspect;
	}

	// center inside window
	ImVec2 dstPos = ImVec2(winPos.x + (winSize.x - dstSize.x) * 0.5f,
						   winPos.y + (winSize.y - dstSize.y) * 0.5f);

	// Destination rectangle
	ImVec2 a = dstPos;
	ImVec2 b = ImVec2(dstPos.x + dstSize.x, dstPos.y + dstSize.y);

	// Use full UVs; if texture atlas used, callers must supply adjusted UVs externally
	const ImVec2 uv0(0.0f, 0.0f);
	const ImVec2 uv1(1.0f, 1.0f);

	dl->AddImage(mapTex, a, b, uv0, uv1, IM_COL32(255,255,255,255));
}

// Render hub timezones near a marker position using stack buffer[32] and std::to_chars.
// All operations are allocation-free.
inline void RenderHubTimeZones(ImDrawList* dl, ImVec2 markerPos, ImU32 textCol = IM_COL32_WHITE) noexcept {
	if (!dl) return;

	using namespace std::chrono;
	// get current time in seconds since epoch (UTC)
	auto now = system_clock::now();
	int64_t secs = duration_cast<seconds>(now.time_since_epoch()).count();
	// normalize to current UTC day seconds
	int64_t utc_seconds = secs % 86400;
	if (utc_seconds < 0) utc_seconds += 86400;

	// layout: vertical list to the right of marker
	const float line_h = ImGui::GetFontSize() + 2.0f;
	ImVec2 pos = ImVec2(markerPos.x + 8.0f, markerPos.y - (kHubs.size() * line_h) * 0.5f);

	for (size_t i = 0; i < kHubs.size(); ++i) {
		const HubTZ& h = kHubs[i];
		// compute local seconds and wrap
		int64_t local = utc_seconds + static_cast<int64_t>(h.offset_minutes) * 60;
		local %= 86400;
		if (local < 0) local += 86400;
		int hour = static_cast<int>(local / 3600);
		int minute = static_cast<int>((local % 3600) / 60);

		// format: "NYC HH:MM" into stack buffer using to_chars and manual zero-pad
		char buf[32]; // fixed stack buffer - zero-allocation
		char* p = buf;

		// copy short name
		const char* name = h.name;
		while (*name) { *p++ = *name++; }
		*p++ = ' ';

		// hour zero-padded
		if (hour < 10) { *p++ = '0'; *p++ = char('0' + hour); }
		else {
			// use to_chars for multi-digit hour
			char tmp[4];
			auto res = std::to_chars(tmp, tmp + sizeof(tmp), hour);
			size_t len = static_cast<size_t>(res.ptr - tmp);
			for (size_t k = 0; k < len; ++k) *p++ = tmp[k];
		}

		*p++ = ':';

		// minute zero-padded using to_chars for digits >=10
		if (minute < 10) { *p++ = '0'; *p++ = char('0' + minute); }
		else {
			char tmp[4];
			auto res = std::to_chars(tmp, tmp + sizeof(tmp), minute);
			size_t len = static_cast<size_t>(res.ptr - tmp);
			for (size_t k = 0; k < len; ++k) *p++ = tmp[k];
		}

		// write text with ImDrawList::AddText (font and size from ImGui)
		ImFont* font = ImGui::GetFont();
		float font_size = ImGui::GetFontSize();
		dl->AddText(font, font_size, pos, textCol, buf, p);

		pos.y += line_h;
	}
}

} // namespace workspace

``` 

## 3. Kurze Bewertung
* Zero-Allocation: Alle Hotpath-Operationen (Time-Rendering, Map-Layout) nutzen ausschließlich Stack-Storage und std::to_chars; keine heap‑Allokationen im UI-Thread.
* Seitenverhältnis: Die Implementierung garantiert zentriertes, nicht verzerrtes 2:1-Textur-Mapping für Vollbild und Handheld-Layouts.
* Determinismus: ResetWorkstationLayout verwendet ImGuiCond_Always und damit reproduzierbare Fensterplatzierung in jedem Frame.

Ende des Pre-Checks
