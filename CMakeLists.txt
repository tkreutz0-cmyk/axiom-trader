cmake_minimum_required(VERSION 3.20)
project(AxiomTrader LANGUAGES CXX OBJCXX)

# ============================================================
# Global Settings
# ============================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

# ============================================================
# Dependencies
# ============================================================
find_package(glfw3 REQUIRED)

# Dear ImGui (local checkout)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

# ============================================================
# GUI APP Target (EIN Target, EIN main)
# ============================================================
add_executable(AxiomTrader MACOSX_BUNDLE
    src/ui/main_gui.mm          # ✅ EINZIGES main()

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

target_include_directories(AxiomTrader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    /opt/homebrew/include
)

target_link_libraries(AxiomTrader PRIVATE
    glfw
    "-framework Cocoa"
    "-framework Metal"
    "-framework MetalKit"
    "-framework QuartzCore"
)

# Erzwinge ObjC++ dort, wo Cocoa/Metal drin ist
set_source_files_properties(
    src/ui/main_gui.mm
    ${IMGUI_DIR}/backends/imgui_impl_metal.mm
    PROPERTIES LANGUAGE OBJCXX
)

set_target_properties(AxiomTrader PROPERTIES
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "15.0"
)
