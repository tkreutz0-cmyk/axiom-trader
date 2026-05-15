#include "gtest/gtest.h"
#include <chrono>
#include "imgui.h"
#include "src/ui/widgets/WorkspaceManager.h"

// We test pure helper functions: ComputeLocalHourMinute and ComputeHubTextPosition

using namespace std::chrono;

TEST(WorkspaceManager_HourMinute, UTCNoOffset) {
	int64_t epoch = 0; // 1970-01-01 00:00:00 UTC
	auto [h,m] = workspace::ComputeLocalHourMinute(epoch, 0);
	EXPECT_EQ(h, 0);
	EXPECT_EQ(m, 0);
}

TEST(WorkspaceManager_HourMinute, MidnightOffset) {
	int64_t epoch = 86399; // one second before day end
	auto [h,m] = workspace::ComputeLocalHourMinute(epoch, 120); // +2 hours -> wraps to 01:59
	EXPECT_EQ(h, 1);
	EXPECT_EQ(m, 59);
}

TEST(WorkspaceManager_Position, Basic) {
	ImVec2 marker{100.0f, 200.0f};
	ImVec2 pos = workspace::ComputeHubTextPosition(marker);
	EXPECT_FLOAT_EQ(pos.x, 80.0f);
	EXPECT_FLOAT_EQ(pos.y, 212.0f);
}
