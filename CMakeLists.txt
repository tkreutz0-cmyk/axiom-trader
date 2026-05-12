cmake_minimum_required(VERSION 3.20)

project(AxiomTrader
    LANGUAGES CXX OBJCXX
)

# ============================================================
# Global Settings
# ============================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Dependencies
# ============================================================
find_package(glfw3 REQUIRED)
find_package(SQLite3 REQUIRED)

# ============================================================
# Dear ImGui (local checkout)
# ============================================================
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

# ============================================================
# GUI APP Target (EIN Target, EIN main)
# ============================================================
add_executable(AxiomTrader MACOSX_BUNDLE
    # Entry point (EINZIGES main)
    src/ui/main_gui.mm

    # Core
    Axiom/Trade.cpp

    # ImGui core
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp

    # ImGui backends
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_metal.mm
)

# ============================================================
# Include Directories (nur Projekt + ImGui)
# ============================================================
target_include_directories(AxiomTrader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ============================================================
# Link Libraries
# ============================================================
target_link_libraries(AxiomTrader PRIVATE
    glfw
    SQLite3::SQLite3
    "-framework Cocoa"
    "-framework Metal"
    "-framework MetalKit"
    "-framework QuartzCore"
)

# ============================================================
# ObjC++ enforcement for Metal / Cocoa files
# ============================================================
set_source_files_properties(
    src/ui/main_gui.mm
    ${IMGUI_DIR}/backends/imgui_impl_metal.mm
    PROPERTIES
        LANGUAGE OBJCXX
)

# ============================================================
# macOS / Xcode specific properties
# ============================================================
set_target_properties(AxiomTrader PROPERTIES
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "15.0"
)
