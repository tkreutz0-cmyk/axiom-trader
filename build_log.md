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
