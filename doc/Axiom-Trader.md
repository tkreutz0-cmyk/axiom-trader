# Project AXIOM-Trader: Master Plan & Architecture

**Author:** Thorsten Kreutz  
**Status:** [1/4] Milestones Completed (Infrastructure & UI Alpha)
**Last Update:** 2026-05-08

---

## 1. VISION
Development of a high-performance, deterministic trading journal based on the **AXIOM concept** (SYNAPSE-CPP kernel). The goal is a platform-independent desktop application following the **"Blender Model"** (no App Store coercion, full GUI sovereignty).

## 2. INFRASTRUCTURE & WORKFLOW
*(Unverändert: Gitea auf NAS, Xcode/CMake, Metal API)*

## 3. TECHNICAL STACK
*(Unverändert: C++20, ImGui, SQLite-Vorbereitung)*

## 4. DIRECTORY STRUCTURE
```text
axiom-trader/
├── docs/           # Master Plan & Architecture Docs
├── src/
│   ├── core/       # AxiomTrade Structures (Current)
│   ├── ui/         # Needle-Dashboard & SYNAPSE-Journal
│   └── platform/   # macOS/Metal Wrapper (Objective-C++)
├── CMakeLists.txt  
└── README.md       
```

## 5. MILESTONES

### 5.1 [x] Setup & Infrastructure
- [x] Initialize Gitea Repo on NAS
- [x] Create CMakeLists.txt for Xcode/Metal
- [x] Automate Documentation Export (Org -> MD)
- [x] **New:** Implement `ImGui_ImplGlfw_InstallCallbacks` for native macOS input.

### 5.2 [ ] AXIOM Core (Headless)
- [x] Define Initial Trade Data Structures in C++20
- [ ] Implement SYNAPSE Engine for deterministic P&L
- [ ] Integrate KS-Validation for Risk Management

### 5.3 [/] UI & Visualization (In Progress)
- [x] Dear ImGui Base Window (Blender-Style)
- [x] Needle-Dashboard Alpha (2D Canvas with Pan & Zoom)
- [x] MacBook Pro Trackpad Optimization (High-Precision Input)
- [ ] Y-Axis Price Transformation (Mapping Price to Pixels)

### 5.4 [ ] Export & Reporting
- [ ] PDF Print Engine for physical journaling

## 6. NOTES
*   **USP Focus:** Die "Needles" im Dashboard sind funktional implementiert (Blau/Rot/Gelb Layer-System).
*   **Retina Support:** Volle DPI-Awareness für MacBook Displays integriert.
