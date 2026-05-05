# ADR-003: Initialisierung der RAD-Umgebung (macOS/M5 Pro)

**Status:** Decided  
**Datum:** 2026-05-04  
**Projekt:** Axiom-Trader  

## 1. Kontext
Für den schnellen Aufbau des Dashboards und der Nadel-Visualisierung wird eine funktionale Entwicklungsumgebung (RAD) auf dem MacBook Pro benötigt. Aufgrund von Port-Hürden und Paket-Verfügbarkeiten wurde ein hybrides Setup gewählt.

## 2. Technische Infrastruktur
- **IDE:** Xcode (via CMake-Generator).
- **Paketmanager:** Homebrew (für System-Abhängigkeiten).
- **Grafik-API:** Metal (Native Apple GPU Beschleunigung).
- **Fenster-Management:** GLFW (via Homebrew).
- **GUI-Library:** Dear ImGui (als lokaler Source-Drop in `external/`).

## 3. CMake Konfiguration (`CMakeLists.txt`)
Die Konfiguration erzwingt C++20 und verknüpft die Homebrew-Pfade von Apple Silicon (`/opt/homebrew`).

```cmake
cmake_minimum_required(VERSION 3.20)
project(AxiomTrader LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

find_package(glfw3 REQUIRED)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

set(PROJECT_SOURCES
    src/main.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_metal.mm
)

add_executable(AxiomTrader MACOSX_BUNDLE ${PROJECT_SOURCES})

target_include_directories(AxiomTrader PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${IMGUI_DIR} 
    ${IMGUI_DIR}/backends
    /opt/homebrew/include
)

target_link_libraries(AxiomTrader PRIVATE 
    glfw
    "-framework Cocoa" "-framework Metal" "-framework MetalKit" "-framework QuartzCore"
)
```

## 4. Workflow-Befehle
1. **Verzeichnisse:** `mkdir -p build external/imgui`
2. **Generierung:** `cd build && cmake -G "Xcode" ..`
3. **Start:** `open AxiomTrader.xcodeproj`

## 5. Nächste Schritte
- [ ] Implementierung der Metal-Initialisierung in `main.cpp`.
- [ ] Test des Rendering-Loops.
- [ ] Dokumentation der ersten erfolgreichen Fenster-Erzeugung in Gitea.
