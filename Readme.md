# AXIOM-Trader (Development Snapshot v0.4.0-alpha)

A high-performance, deterministic trading journal for macOS, built with a "Desktop-First" philosophy.

## 🚀 The Vision
AXIOM-Trader follows the **"Blender Model"**: full UI sovereignty, zero App Store friction, and maximum performance. Unlike mobile-centric apps, this project treats macOS as a professional workstation, utilizing a hybrid C++20 and Metal API stack.

## 🛠 Technical Stack
- **Core:** C++20 (Deterministic simulation and logic)
- **Graphics:** Metal API (Native Apple Silicon acceleration via `CAMetalLayer` Hosting)
- **GUI:** Dear ImGui (v1.9x) + GLFW
- **Architecture:** Specialized **Layer-Hosting View** for Zero-Latency Rendering.

## 📂 Current Features (Snapshot v0.4.0)
- **AXIOM World Engine:** 
    - **Geospatial Mapping:** Core database for global trading hubs (NYC, LDN, FRA, TYO, SYD).
    - **Symbol-Interaction:** High-precision hit-detection (Euclidean distance check) for direct "Load-to-Edit" workflows.
    - **Visual Hashing:** Consistent asset-coloring via symbol-based procedural hashing.
- **SYNAPSE Master-Control:** 
    - Integrated CRUD-system for trade management.
    - Directional UI cues: **LONG (▲)** and **SHORT (▼)**.
- **Layout Manager:** Integrated grid-reset logic to maintain workstation workspace stability.
- **Retina Precision:** Synchronized framebuffer scaling against UI blur and freezing.

## 🐛 Recent Hotfixes (v0.4.0)
- Fixed **Event-Occlusion** in trade tables via improved ID-scoping.
- Resolved **Memory Overflows** in string buffering using safe `snprintf` handling.
- Stabilized **Metal Render Pass** descriptors via explicit index addressing.

## 📅 Roadmap (v0.5.x)
- [ ] **Infrastructure:** SQLite-Integration for deterministic trade persistence.
- [ ] **Visuals:** `stb_image.h` integration for high-res PNG map textures.
- [ ] **Analytics:** Real-time global PnL aggregation and equity curve visualization.

---
**Status:** `Checkpoint v0.4.0-alpha - Build Stable`  
**Platform:** macOS 15.x (Apple Silicon Optimized)

# AXIOM-Trader (Development Snapshot v0.4.0-alpha)

![Build Status](github.com)
[![Platform: macOS](https://shields.io)](https://github.com/tkreutz0-cmyk/axiom-trader)
[![Language: C++20](https://shields.io)](https://github.com/tkreutz0-cmyk/axiom-trader)

A high-performance, deterministic trading journal for macOS, built with a "Desktop-First" philosophy.

