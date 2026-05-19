# AXIOM-Trader

> A deterministic, high-performance trading journal built with C++20, Dear ImGui, and Metal.

---

## ✨ Overview

**AXIOM-Trader** is a desktop-first trading journal designed for performance, determinism, and architectural clarity.

Unlike conventional tools, AXIOM enforces a **strict separation between domain logic and UI**, ensuring that all trading calculations remain reproducible across platforms and runs.

The system follows a **simulation-first philosophy**:
> Not “what could happen” — but “what is allowed by the system’s constraints”.

---

## 🧠 Core Architecture

AXIOM-Trader is built around a **deterministic domain kernel**:

- ✅ Pure C++20 domain logic (platform-independent)
- ✅ Strict UI decoupling (no ImGui/Metal in core)
- ✅ Reproducible calculations (PnL, pips, clustering)
- ✅ Explicit data flow:  
  **SQLite → Core → Transformation → UI → Rendering**

Key principles:

- Determinism over heuristics  
- Zero overhead in hotpaths  
- Architecture enforced via ADRs  

---

## 🗺 AXIOM World Engine

### ✅ Stage D1 Milestone (Completed)

The full rendering pipeline is now operational.

#### Pipeline Status
- Trades are loaded from SQLite at startup

### UI & Workspace Management
Das System unterstützt ein automatisiertes, symmetrisches Fenster-Arrangement (Grid-Verhältnis: 25% | 50% | 25%). 

- **Bedienung**: Klicken Sie im Hauptfenster ("AXIOM Trader") unter den *PnL Settings* auf den Button **"Layout anordnen"**.
- **Entwickler-Hinweis**: Neue ImGui-Fenster müssen über `workspaceManager.BeginWindow(WorkspaceManager::WindowId::...)` registriert werden, anstatt das native `ImGui::Begin()` zu nutzen, damit sie vom Layout-Grid erfasst werden.
