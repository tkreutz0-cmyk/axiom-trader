cmake_minimum_required(VERSION 3.20)
project(axiom-trader-tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add GoogleTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(workspace_manager_tests
	tests/WorkspaceManagerTests.cpp
	src/ui/widgets/WorkspaceManager.mm
)

target_include_directories(workspace_manager_tests PRIVATE src)

target_link_libraries(workspace_manager_tests PRIVATE GTest::GTest GTest::Main)

enable_testing()
add_test(NAME WorkspaceManagerTests COMMAND workspace_manager_tests)
