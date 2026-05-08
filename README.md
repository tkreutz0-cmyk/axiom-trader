# AXIOM-Trader (Development Snapshot v0.4.0-alpha)
...
## 📂 Current Features (Snapshot v0.4.0)
Verwende Code mit Vorsicht.Für das BUILD_LOG.md:markdown# AXIOM TRADER - Project Build Log

## DATEI-INFO
*   **Projektname:** AXIOM Trader
*   **Version:** v0.4.0-alpha (Milestone: Metal & Geometry Stable)
Verwende Code mit Vorsicht.Für den Git-Kommentar (angepasst):bashgit commit -m "feat: stabilize Metal core & add interactive trading journal v0.4.0-alpha

- Fix: Critical macOS 15 black screen via layer-hosting synchronization
- Engine: New Ear-Clipping triangulation & normalization pipeline
- World: Aspect-ratio corrected high-fidelity vector map
- UI: Interactive trade editing & UTC-chrono mapping"
Verwende Code mit Vorsicht.Sollen wir als nächsten Schritt die Markt-Sessions (London/NY Glow-Zonen) angehen, um die v0.4.x-Reihe abzuschließen?KI-Antworten können Fehler enthalten. Weitere InformationenFrage dazu stellen

A high-performance, deterministic trading journal for macOS, built with a "Desktop-First" philosophy.

## 🚀 The Vision
AXIOM-Trader follows the **"Blender Model"**: full UI sovereignty, zero App Store friction, and maximum performance. Unlike mobile-centric apps, this project treats macOS as a professional workstation, utilizing a hybrid C++20 and Metal API stack.

## 🛠 Technical Stack
- **Core:** C++20 (Deterministic simulation and logic)
- **Graphics:** Metal API (Native Apple Silicon acceleration via Layer-Hosting)
- **Geometry:** Custom Ear-Clipping Triangulation Engine for concave polygons.
- **GUI:** Dear ImGui (Immediate Mode UI for high responsiveness)
- **Windowing:** GLFW (Robust Cocoa integration)

## 🏗 Architecture Highlight: The AXIOM-Metal Bridge
To ensure professional standards on macOS 15.x, the project uses a specialized **Layer-Hosting View** architecture. By strictly managing the `CAMetalLayer` within an Objective-C++ (`.mm`) wrapper, AXIOM achieves:
- **Zero-Latency Rendering:** Direct access to the Apple Silicon GPU.
- **Retina Precision:** Dynamic synchronization between `view.bounds` and `layer.drawableSize`.
- **Persistence Foundation:** Proper path handling in `~/Library/Application Support`.

## 📂 Current Features (Snapshot v35.0)
- **SYNAPSE Master-Control:** Interactive trade entry and editing system with real-time state synchronization.
- **AXIOM World Engine:** A high-fidelity vector world map with 2:1 aspect ratio correction (Letterboxing).
- **Geometry Pipeline:** Automated vertex normalization and CCW-winding enforcement for artifact-free rendering.
- **Chrono-Mapping:** Trades are visualized as geographic anchors on the world map based on UTC timestamps.
- **DPI-Awareness:** Full Retina-display support with automated framebuffer scaling.

## 📝 Usage & Controls
- **Select Trade:** Click any entry in the Master Table to load data into the editor.
- **Update/Edit:** Modify trade parameters and sync them back to the core vector.
- **Pan/Zoom:** Two-finger scroll and `Option` + Scroll (Dashboard optimized).
- **Status Monitoring:** Real-time UTC-time tracking via the "Global Session Monitor".

---
**Status:** Milestone 8.0 (Metal Stability & Vector Geometry) complete.  
**Next Up:** Persistence Layer (SQLite) & Market Session Glow-Zones (LDN/NY/TYO).
