# Build Log

## 2026-05-12 — Core merge cleanup & build stabilization

### Context
During a merge of two repositories, duplicate implementations of
`Axiom::Trade` were introduced into the build.

### Changes
- Removed `AxiomTrade.cpp`
- Kept `Trade.hpp` + `Trade.cpp` as the single source of truth
- Ensured exactly one translation unit defines `Axiom::Trade`

### Rationale
Having multiple `.cpp` files defining the same class violates the
C++ One Definition Rule (ODR) and can lead to:
- duplicate symbol linker errors
- undefined behavior depending on link order

This commit restores a deterministic, well-defined core build state
and serves as a clean baseline for upcoming persistence (SQLite DAO)
work.

### Status
- ✅ Build clean
- ✅ Core logic deterministic
- ✅ Ready for DAO integration (Trade / AssetSpec)

## 2026‑05‑12 — Stage D: SQLite persistence stabilization

### Summary
- Completed Stage D core persistence integration.
- Database is now created automatically by SQLite on first run.
- AssetSpec and Trade persistence initialized deterministically at startup.

### Technical Details
- Refactored SQLite wrapper (`Sqlite.hpp`) to be header‑only and free of side effects.
- Removed premature sqlite3_prepare_v2 calls from DAO constructors.
- Introduced lazy statement preparation triggered after `ensureSchema()`.
- Applied identical fix pattern to both `AssetSpecDao` and `TradeDao`.
- Added detailed SQLite error context (rc + errstr + errmsg) for debugging.

### Result
- ✅ Build clean
- ✅ Application starts without crashes
- ✅ Tables `AssetSpec` and `Trade` are created automatically
- ✅ Persistence layer ready for further extension (Async / Worker model)

### Next Steps (Deferred)
- Optional: Stage D2 — async DB worker to remove I/O from UI thread
- Optional: GUI‑level error reporting instead of abort on DB errors



## [v0.4.0-alpha] - 2026-05-13:
- Vorbereitung der persistenten SQLite-Datenbank
- Test des M365 Copilot PO-Workflows

---

## Build Log (macOS / Metal / CMake)

### 2026‑05‑14 — Stage D1: Trading Map Infrastructure

**Status:** ✅ stable

Changes / Fixes:
- Added `MapTexture` RAII wrapper (stb_image + Metal `MTLTexture`)
- Fixed Objective‑C++ ARC bridging (`__bridge void*`) for Metal device handoff
- Registered `MapTexture.mm` explicitly in CMake target to avoid linker errors
- Integrated `stb_image.h` as external single‑header dependency
- Added `assets/world_map.png` as **macOS bundle resource** via
  `MACOSX_PACKAGE_LOCATION Resources`
- World map now loads via `NSBundle mainBundle` and renders in a dedicated
  ImGui window (“Trading Map”)

Notes:
- Assets inside `assets/` are **not available at runtime** unless explicitly
  added to the app bundle.
- When using `MACOSX_BUNDLE`, all runtime assets must be registered via CMake
  `target_sources(...)` + `MACOSX_PACKAGE_LOCATION`.

Known limitations:
- Map currently renders static (no zoom / pan)
- No trade markers yet (planned for Stage D2)

## 2026-05-19 — Stage D1 Completion: World Map Rendering Pipeline

### Context
The application previously suffered from a "Silent Failure" state:  
Trade data was correctly loaded and processed in the deterministic core but was not visualized due to a missing integration step in the GUI render loop.

### Changes
- Integrated `Axiom::buildClusters()` into the ImGui render loop (`main_gui.mm`)
- Implemented stable world map rendering pipeline:
  - `ImGui::Image()` for base map
  - `ImDrawList` overlay for trade markers
- Removed broken `WorldRenderer` linkage (undefined symbol issue)
- Fixed `ImTextureID` comparison (`nullptr` → `0`)
- Stabilized ImGui render order and coordinate mapping

### Result
- ✅ World map renders correctly
- ✅ Trades are displayed at correct geospatial locations
- ✅ No more "Silent Failure"
- ✅ Build clean and stable

### Architectural Impact
- Pipeline between **Core → UI → Rendering** is now closed
- Deterministic data flow verified end-to-end
- Foundation established for Stage D2 (interaction, clustering UI, async DB)

### Status
✅ Milestone complete  
✅ System visually and functionally coherent  
✅ Ready for next stage (interaction layer / DB worker)


===================================================================
BUILD LOG: RE-ENABLING WORKSPACE ARRANGEMENT (Stage D1.1)
===================================================================
- Task: Restore automatic window layout anchoring via WorkspaceManager.
- Technical Fixes:
  * Resolved 'Use of undeclared identifier' by shifting manager instantiation to UI state block.
  * Corrected 'axiom::ui' namespace error (WorkspaceManager resides in global axiom/root scope).
  * Shipped strongly-typed window binding via WorkspaceManager::WindowId enums.
  * Disambiguated WindowId conflict against standard int main() using explicit typing.
- Status: Build Succeeded. Symmetrical layout grid triggers correctly on button press.

