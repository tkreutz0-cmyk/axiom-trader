cmake_minimum_required(VERSION 3.20)

# ============================================================
# Plattformunabhängige Projektschichten (Blender-Modell)
# ============================================================
if(APPLE)
    project(AxiomTrader LANGUAGES CXX OBJCXX)
else()
    project(AxiomTrader LANGUAGES CXX)
endif()

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
# GUI APP Target (Plattformunabhängige Basis)
# ============================================================
add_executable(AxiomTrader)

# Gemeinsame Core-Quelldateien (Plattformunabhängig)
target_sources(AxiomTrader PRIVATE
    Axiom/Trade.cpp
    Axiom/AssetSpecDao.cpp
    Axiom/TradeDao.cpp
    Axiom/StorageManager.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

# Inklusions-Pfade für alle Plattformen
target_include_directories(AxiomTrader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Link-Bibliotheken für alle Plattformen
target_link_libraries(AxiomTrader PRIVATE
    glfw
    SQLite3::SQLite3
)

# ============================================================
# macOS / Xcode spezifisches Target-Hosting
# ============================================================
if(APPLE)
    set_target_properties(AxiomTrader PROPERTIES MACOSX_BUNDLE ON)

    # macOS-spezifische Render- und UI-Dateien anhängen
    target_sources(AxiomTrader PRIVATE
        src/ui/main_gui.mm
        src/ui/MapTexture.mm
        ${IMGUI_DIR}/backends/imgui_impl_metal.mm
    )

    # Assets als macOS App-Ressourcen bündeln
    set(AXIOM_ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets/world_map.png)
    set_source_files_properties(${AXIOM_ASSETS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(AxiomTrader PRIVATE ${AXIOM_ASSETS})

    # Frameworks für Apple Metal anbinden
    target_link_libraries(AxiomTrader PRIVATE
        "-framework Cocoa"
        "-framework Metal"
        "-framework MetalKit"
        "-framework QuartzCore"
    )

    # Objective-C++ Compiler-Erzwingung für Apple Clang
    set_source_files_properties(
        src/ui/MapTexture.mm
        src/ui/main_gui.mm
        ${IMGUI_DIR}/backends/imgui_impl_metal.mm
        PROPERTIES LANGUAGE OBJCXX
    )

    # Xcode Deployment und Code-Signing-Eigenschaften
    set_target_properties(AxiomTrader PROPERTIES
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
        XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "15.0"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED YES
        XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED YES
    )
endif()
