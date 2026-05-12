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
