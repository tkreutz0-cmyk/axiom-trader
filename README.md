# AXIOM-Trader (Alpha Snapshot v0.1)

A high-performance, deterministic trading journal for macOS, built with a "Desktop-First" philosophy.

## 🚀 The Vision
AXIOM-Trader follows the **"Blender Model"**: full UI sovereignty, zero App Store friction, and maximum performance. Unlike mobile-centric apps, this project treats macOS as a professional workstation, utilizing a hybrid C++20 and Metal API stack.

## 🛠 Technical Stack
- **Core:** C++20 (deterministic simulation and logic)
- **Graphics:** Metal API (Native Apple Silicon acceleration)
- **GUI:** Dear ImGui (Immediate Mode UI for high responsiveness)
- **Windowing:** GLFW (Robust Cocoa integration)

## 🏗 Architecture Highlight: The Hybrid Bridge
To ensure professional standards on macOS (e.g., proper path handling in `~/Library/Application Support`), the project uses **Objective-C++ (.mm)** wrappers. This bridges the gap between C++ logic and native Foundation frameworks.

## 📂 Current Features (Snapshot v0.1)
- **SYNAPSE-Journal:** Manual trade entry system (Core Module 1).
- **Needle-Dashboard:** Dynamic 2D-Canvas for trade visualization (Module 2).
- **MacBook Pro Optimization:** Custom Trackpad-Handling for high-precision Panning and Zooming (`Option` + Scroll).
- **DPI-Awareness:** Full Retina-display support via framebuffer scaling.

## 📝 Usage & Controls
- **Pan:** Two-finger scroll on dashboard.
- **Zoom:** `Option` (Alt) + Two-finger scroll.
- **Add Trade:** Use the SYNAPSE-Journal window to push data to the core vector.

---
**Status:** Milestone 5.2 (UI-Infrastructure & Canvas Alpha) complete.  
**Next Up:** SYNAPSE-CPP Kernel & Y-Axis price transformation.
