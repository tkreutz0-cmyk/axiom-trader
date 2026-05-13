# Domain Kernel – Axiom-Trader

## Purpose
The Domain Kernel represents the deterministic, platform-agnostic core of
Axiom-Trader. It contains the essential business rules and data models required
to describe trades, assets, and their lifecycle independently of any
blockchain, transport, UI, or execution environment.

This layer is designed to be stable over time and reusable across multiple
frontends (macOS, iOS, iPadOS, Windows, Linux) and execution engines
(e.g. Rust-based strategy engines or external APIs).

---

## Responsibilities
- Trade lifecycle representation (entry → exit → settlement)
- Deterministic calculations (PnL, pips, symbolic color mapping)
- Asset specifications (pip size, contract size, asset type)
- Persistence abstraction and guarantees
- Strict enforcement of domain invariants

---

## Non-Responsibilities
The Domain Kernel must **not** contain:
- Network or blockchain I/O
- Wallet management, signing, or key storage
- Authentication, authorization, or identity logic
- UI, rendering, or platform-specific code
- Heuristic or strategy-specific trading logic

---

## Invariants
- Deterministic behavior across platforms and runs
- One Definition Rule (ODR) respected at all times
- No hidden side effects in constructors
- No implicit heuristics or symbol-based assumptions
- Explicit handling of nullable domain states (e.g. open trades)

---

## Time Semantics
All time values stored or processed by the Domain Kernel are defined as:
- **Unix epoch timestamps**
- Resolution must be consistent across the system
  (e.g. seconds or milliseconds, but never mixed)

The exact resolution must be documented and enforced uniformly.

---

## Quantity Semantics
Trade quantities must have a clear and unambiguous meaning:
- Either expressed in **lots** (FX-style)
- Or expressed in **units/contracts**, combined with AssetSpec metadata

The Domain Kernel must not silently convert between these representations.

---

## Evolution Rules
- Changes to the Domain Kernel must be incremental and testable
- Public data structures are treated as long-lived contracts
- Schema or enum changes must consider backward compatibility
- All new logic must be covered by deterministic tests where applicable

---

## Relation to Other Layers
- Upper layers (API, Execution Engine, UI) depend on the Domain Kernel
- The Domain Kernel must not depend on any upper layer
- Interoperability is achieved via explicit interfaces or adapters

This separation ensures long-term maintainability, portability, and
architectural clarity.