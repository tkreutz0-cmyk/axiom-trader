# ADR-001: Domain Kernel as Deterministic Core

## Status
Accepted

---

## Context
Axiom-Trader is designed to operate across multiple platforms
(macOS, iOS, iPadOS, Windows, Linux) and multiple execution environments
(API servers, strategy engines, blockchain integrations).

Early development has shown the necessity of a stable architectural core
that is independent of:
- blockchain protocols and RPC semantics
- networking, WebSockets, or REST APIs
- UI frameworks and platform-specific concerns
- execution strategies and heuristics

Without an explicitly defined core, there is a high risk of:
- duplicated business logic across layers
- implicit heuristics leaking into fundamental calculations
- platform-dependent behavior and hard-to-reproduce bugs
- long-term architectural erosion.

---

## Decision
We define and enforce a **Domain Kernel** as a deterministic,
technology-agnostic core of the Axiom-Trader system.

The Domain Kernel:
- models trades, assets, and their lifecycle
- performs deterministic calculations (e.g. PnL, pips)
- defines domain invariants and semantic rules
- owns persistence abstractions for domain data

The Domain Kernel must not depend on any execution engine, blockchain,
API layer, or UI technology.

---

## Rationale
This decision establishes a clear architectural center of gravity.

Determinism ensures that:
- calculations are reproducible across platforms and runs
- behavior can be tested and reasoned about in isolation
- bugs are localized and traceable

Technology agnosticism ensures that:
- execution engines can evolve independently (e.g. C++ → Rust)
- UI implementations can vary without affecting core logic
- blockchain integrations remain adapters, not authorities

Alternative approaches were considered:
- Embedding domain logic directly in execution engines  
  → rejected due to coupling, duplication, and poor testability
- Allowing heuristic shortcuts in the core  
  → rejected due to loss of determinism and auditability

---

## Consequences
### Positive
- Clear separation of responsibilities across system layers
- Long-lived, stable domain contracts
- Improved testability and reasoning
- Safer refactoring and platform expansion

### Negative
- Additional upfront discipline when adding new features
- Some logic must be repeated or adapted in higher layers
  (by design, via explicit interfaces)

### Follow-up
- All existing and future core logic must be reviewed
  against Domain Kernel responsibilities
- Violations must be refactored into adapters or upper layers

---

## Implications for Other Layers
- **Execution / Strategy Engine:** consumes Domain Kernel models,
  but does not redefine their semantics
- **API / Integration Layer:** exposes domain state without mutating
  domain rules
- **UI Layers:** render domain data but never encode business logic
- **Persistence:** Domain Kernel defines schema meaning; storage
  technology may vary

---

## Review / Re-evaluation Criteria
This decision should be revisited only if:
- Domain concepts fundamentally change
- Determinism becomes technically infeasible
- The system scope is reduced to a single platform and context

In absence of these conditions, this ADR is intended to be long-lived.

---

## References
- doc/architecture/domain-kernel.md
- ADR-000: Architectural Decision Record Template

