# ADR-000: Architectural Decision Record Template

## Status
Proposed | Accepted | Deprecated | Superseded

---

## Context
Describe the architectural context that led to this decision.

Include:
- The problem or constraint being addressed
- Relevant system boundaries (Domain Kernel, Execution Engine, API, UI, etc.)
- Non-functional requirements (latency, security, determinism, portability)
- Any prior decisions this builds upon or conflicts with

This section must be factual and not contain the decision itself.

---

## Decision
State the architectural decision clearly and concisely.

The decision must:
- Be specific and unambiguous
- Describe *what* is chosen, not how it is implemented
- Avoid justifying language (that belongs in Consequences)

Example:
> We will treat the Domain Kernel as a deterministic, platform-agnostic core
> with no dependencies on blockchain, networking, or UI layers.

---

## Rationale
Explain **why** this decision was made.

Include:
- Key trade-offs considered
- Alternative options that were explicitly rejected
- Reasons for rejecting those alternatives

This section captures the architectural thinking so it does not need
to be rediscovered later.

---

## Consequences
Describe the consequences of this decision.

Include:
- Positive outcomes
- Negative impacts or limitations
- New constraints that follow from this decision
- Required follow-up actions (e.g. refactors, new interfaces, documentation)

Be honest and explicit — this section is critical for long-term maintainability.

---

## Implications for Other Layers
Describe how this decision affects:
- Domain Kernel
- Execution / Strategy Engine
- API / Integration Layer
- UI / Platform-specific layers
- Tooling, testing, or deployment

---

## Review / Re-evaluation Criteria
Specify when this decision should be revisited.

Examples:
- Significant performance regression
- New target platform or chain
- Architectural coupling issues
- Major change in system scope

---

## References
List related documents, ADRs, issues, or commits.

Examples:
- ADR-001: Domain Kernel Definition
- DOC/architecture/domain-kernel.md
- Relevant commits or pull requests

