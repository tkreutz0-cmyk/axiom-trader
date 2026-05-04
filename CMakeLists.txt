cmake_minimum_required(VERSION 3.20)
project(AxiomTrader LANGUAGES CXX OBJCXX)

# C++20 Standard erzwingen
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Homebrew-Pfade für Apple Silicon (M5) priorisieren
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

# 2. Suche GLFW (via Homebrew installiert)
find_package(glfw3 REQUIRED)

# 3. Pfade für Dear ImGui (dein lokaler Source-Drop)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

# 4. Definition der Quellcode-Dateien
set(PROJECT_SOURCES
    src/main.mm
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_metal.mm
)

# 5. Das Executable als macOS App-Bundle definieren
add_executable(AxiomTrader MACOSX_BUNDLE ${PROJECT_SOURCES})

# 6. Include-Verzeichnisse (Wo liegen die Header?)
target_include_directories(AxiomTrader PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${IMGUI_DIR} 
    ${IMGUI_DIR}/backends
    /opt/homebrew/include
)

# 7. Frameworks und Libraries verknüpfen
target_link_libraries(AxiomTrader PRIVATE 
    glfw
    "-framework Cocoa" 
    "-framework Metal" 
    "-framework MetalKit" 
    "-framework QuartzCore"
)

# 8. Xcode-spezifische Einstellungen (für Metal-Unterstützung)
set_target_properties(AxiomTrader PROPERTIES
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "15.0"
)
