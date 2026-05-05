# AXIOM-Trader (Alpha Snapshot v0.1)

A high-performance, deterministic trading journal for macOS, built with a "Desktop-First" philosophy.

## 🚀 The Vision
AXIOM-Trader follows the **"Blender Model"**: full UI sovereignty, zero App Store friction, and maximum performance. Unlike mobile-centric apps, this project treats macOS as a professional workstation, utilizing a hybrid C++20 and Metal API stack.

## 🛠 Technical Stack
- **Core:** C++20 (for deterministic simulation and logic)
- **Graphics:** Metal API (Native Apple Silicon acceleration)
- **GUI:** Dear ImGui (Immediate Mode UI for high responsiveness)
- **Windowing:** GLFW (Robust Cocoa integration)

## 🏗 Architecture Highlight: The Hybrid Bridge
To ensure professional standards on macOS (e.g., proper path handling in `~/Library/Application Support`), the project uses **Objective-C++ (.mm)** wrappers. This bridges the gap between C++ logic and native Foundation frameworks.

## 📂 Project Structure
- `/src`: Core C++20 logic and Metal/ImGui implementation.
- `/docs`: Technical Whitepaper (Architecture & Design).
- `CMakeLists.txt`: Build configuration for macOS/Xcode.

## 📝 Note on this Repository
This is a **manual snapshot** exported from a private Gitea instance. It represents the current state of the architecture and the "Project Anchor" implementation.

---
**Status:** Milestone 5.1 (Setup & Infrastructure) complete.  
**Next Up:** SYNAPSE-CPP Kernel for deterministic P&L calculations.
