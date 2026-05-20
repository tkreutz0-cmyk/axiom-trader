cmake_minimum_required(VERSION 3.20)
project(AxiomTrader LANGUAGES CXX OBJCXX)

# C++20 Standard erzwingen laut ADR-001
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Homebrew Pfade für Apple Silicon hinzufügen
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

# Abhängigkeiten finden
find_package(glfw3 REQUIRED)

# Verzeichnisse definieren
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

# Bereinigte Quellcodedateien
# ✅ KORREKTUR: src/main.mm entfernt, um den duplicate symbol '_main' Fehler zu eliminieren!
set(PROJECT_SOURCES
    src/ui/main_gui.mm
    src/ui/MapTexture.mm
    Axiom/Trade.cpp
    Axiom/TradeDao.cpp
    Axiom/AssetSpecDao.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_metal.mm
)

# App Bundle erstellen
add_executable(AxiomTrader MACOSX_BUNDLE ${PROJECT_SOURCES})

# Ressourcen (Weltkarte) ins macOS App-Bundle kopieren
target_sources(AxiomTrader PRIVATE assets/world_map.png)
set_source_files_properties(assets/world_map.png PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
)

# Include-Verzeichnisse setzen
target_include_directories(AxiomTrader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Axiom
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    /opt/homebrew/include
)

# Linker-Bibliotheken und macOS Frameworks zuweisen
target_link_libraries(AxiomTrader PRIVATE
    glfw
    sqlite3
    "-framework Cocoa"
    "-framework Metal"
    "-framework MetalKit"
    "-framework QuartzCore"
)

