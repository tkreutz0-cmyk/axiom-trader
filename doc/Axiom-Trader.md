# Project AXIOM-Trader: Master Plan & Architecture

**Author:** Thorsten Kreutz  
**Status:** [0/4] Milestones Completed  
**Last Update:** 2026-05-04

---

## 1. VISION
Development of a high-performance, deterministic trading journal based on the **AXIOM concept** (SYNAPSE-CPP kernel). The goal is a platform-independent desktop application following the **"Blender Model"** (no App Store coercion, full GUI sovereignty).

## 2. INFRASTRUCTURE & WORKFLOW

### 2.1 Source Control
*   Private Repository on NAS (via Gitea).
*   Mirror/Backup strategy for public snapshots.

### 2.2 Documentation Flow
*   **Master Planning:** Org-Mode (local on M5 Pro).
*   **Technical Documentation:** Markdown (Exported to `/docs`).

### 2.3 Development Environment (M5 Pro)
*   **IDE:** Xcode (via CMake Xcode generator).
*   **Compiler:** Apple Clang (C++20).
*   **Graphics:** Metal API (via Dear ImGui).

## 3. TECHNICAL STACK


| Component | Technology |
| :--- | :--- |
| **Core (Backend)** | C++20 (Deterministic Simulation Graphs) |
| **Logic Validation** | KS-System (Constitutional Stability) |
| **UI-Framework** | Dear ImGui (Immediate Mode GUI) |
| **Build-System** | CMake (Cross-platform) |
| **Database** | SQLite (Local encrypted file) |

## 4. DIRECTORY STRUCTURE
```text
axiom-trader/
├── docs/           # Markdown Documentation
├── src/
│   ├── core/       # SYNAPSE-CPP (Trading Logic)
│   ├── ui/         # ImGui Widgets (Needle-Dashboard)
│   ├── platform/   # macOS/Metal Wrapper
│   └── include/    # Header Files
├── external/       # Dear ImGui, SQLite, etc.
├── CMakeLists.txt  # Central Build Script
└── README.md       # Project Overview
```

## 5. MILESTONES

### 5.1 [ ] Setup & Infrastructure
- [x] Initialize Gitea Repo on NAS
- [x] Create CMakeLists.txt for Xcode/Metal
- [x] Automate Documentation Export (Org -> MD)

### 5.2 [ ] AXIOM Core (Headless)
- [ ] Define Trade Data Structures in C++20
- [ ] Implement SYNAPSE Engine for P&L
- [ ] Integrate KS-Validation for Risk Management

### 5.3 [ ] UI & Visualization
- [ ] Dear ImGui Base Window (Blender-Style)
- [ ] Needle-Dashboard (2D Canvas with Zoom)
- [ ] Chart Component (Inspired by Lotus Organizer)

### 5.4 [ ] Export & Reporting
- [ ] PDF Print Engine for physical journaling

## 6. NOTES
*   Focus on independence from OS versions.
*   The "Needles" in the dashboard are the USP compared to standard journals.
