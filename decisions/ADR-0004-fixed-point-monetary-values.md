# Architecture Decision Record (ADR)

## ADR-0004: Striktes Verbot von Fließkommazahlen für Geldwerte und Einführung von Fixed-Point-Arithmetik

* **Status:** Akzeptiert
* **Datum:** 15. Mai 2026
* **Kontext:** Community-Feedback (Fachinformatiker-Forum v0.4.0-alpha) & Vorbereitung der SQLite-Persistenz (v0.5.x)
* **Architekt:** tkreutz0-cmyk

---

## 1. Kontext und Problemstellung
Im aktuellen Snapshot v0.4.0-alpha des *AXIOM Trader* wurden Währungsbeträge, Asset-Preise und Kontobalancen temporär als standardmäßige C++ Fließkommazahlen (`float` / `double`) verarbeitet. 

Dies stellt im Kontext eines deterministischen Handelssystems ein kritisches Risiko dar. Fließkommazahlen nach IEEE 754 weisen systembedingte Rundungsfehler bei periodischen Dezimalbrüchen auf. Im Verlauf von fortlaufenden Transaktionen und PnL-Aggregationen führt dies zu mathematischen Diskrepanzen (Loss of Precision).

Da das System im Sinne des **"Blender-Modells"** langfristig vollständig plattformübergreifend (macOS via Xcode/Metal, Windows via VS2022, Linux) agieren soll, muss die mathematische Integrität im Kern absolut deterministisch sein.

## 2. In Betracht gezogene Optionen
* **Option A:** Beibehaltung von `double` unter Verwendung von Rundungs-Hilfsfunktionen. (Verworfen, da fehleranfällig und Performance-Overhead).
* **Option B:** Integration externer Big-Decimal-Bibliotheken (z. B. Boost.Multiprecision). (Verworfen, um den Core maximal leichtgewichtig, autark und frei von externen Abhängigkeiten zu halten).
* **Option C:** Implementierung einer nativen, plattformunabhängigen Fixed-Point-Struktur auf Basis von `int64_t` (Micro-Cents mit 4 festen Dezimalstellen). (Gewählt).

## 3. Entscheidung
Es wird ein unumstößliches Verbot für den Datentyp `float` und `double` bezüglich sämtlicher monetärer Werte im gesamten Repository erlassen.

Alle Preise und Geldbeträge werden ausnahmslos über eine plattformunabhängige C++20-Struktur namens `FixedPoint` abgewickelt. Als interne Repräsentation dient ein vorzeichenbehafteter 64-Bit-Ganzzahltyp (`int64_t`).

### Technische Vorgaben für die Code-Generierung (Sidecar-KI):
* Die Struktur wird plattformunabhängig in `src/core/utils/fixed_point.hpp` hinterlegt.
* Basis-Skalierung: 4 Dezimalstellen (1.0000 entspricht dem internen Ganzzahl-Wert `10000`).
* Sämtliche arithmetischen Operatoren (`+`, `-`, `*`, `/`) müssen über C++ Operator-Overloading abgebildet werden.
* Überläufe (Overflows) müssen in Debug-Builds via `assert` abgesichert werden.

## 4. Konsequenzen
* **Positiv:** Absolute mathematische Deterministik über alle Compiler und OS-Plattformen hinweg.
* **Positiv:** Zero-Overhead im Vergleich zu komplexen Klassenstrukturen; hervorragend für den CPU-Hotpath geeignet.
* **Negativ:** Bestehender Code in `src/core/` muss in einem großflächigen Refactoring-Schritt angepasst werden.
* **Einfluss auf die Roadmap (v0.5.x):** Die kommende Integration von SQLite muss Beträge zwingend nur noch als Ganzzahlen (`INTEGER`) in die Datenbank schreiben.

---

## 5. Architektur-Update: Compiler-Souveränität & Cross-Plattform-DNA (Post-Pre-Check)

### Kontext

Im Rahmen eines automatisierten Pre-Checks (docs/pre-checks/0004-fixed-point-review.md) wurde ein kritischer Architekturbruch identifiziert:

- Die Implementierung nutzte implizit `__int128` für interne Berechnungen
- `__int128` ist **kein Bestandteil des C++-Standards**
- MSVC (Windows) unterstützt diesen Typ **nicht**

Dies führt zu einem plattformabhängigen Verhalten und verletzt das grundlegende Architekturprinzip des Systems.

---

### Architekturregel (neu etabliert)

> Der Core darf ausschließlich Sprach- und Compiler-Features verwenden, die:
> - entweder standardisiert sind
> - oder explizit über plattformabhängige Abstraktionen abgesichert werden

---

### Technische Anpassung

- `int64_t` bleibt der definierte Datenraum für Persistenz
- 128-Bit-Arithmetik wird **nicht mehr implizit vorausgesetzt**
- Stattdessen erfolgt eine explizite Behandlung:

#### Plattformabhängige Implementierung

- Clang / GCC:
  - Verwendung von `__int128` als Intermediate

- MSVC:
  - Verwendung von x64-Intrinsics:
    - `_umul128`
    - `_udiv128`

#### Verpflichtende Compiler-Guards

Alle kritischen Arithmetic-Operationen müssen über Compiler-Guards abgesichert werden.

---

### Begründung

Diese Anpassung ist keine Optimierung, sondern die Korrektur eines architektonischen Regelbruchs.

Der Pre-Check hat gezeigt, dass:

- Plattformabhängigkeit auch durch Compiler-Erweiterungen entsteht
- solche Fehler erst spät im Build-Prozess sichtbar werden

Durch die explizite Absicherung wird:

- deterministisches Verhalten sichergestellt
- Cross-Platform-Support garantiert

---

### Leitprinzip (explizit gemacht)

> Plattformunabhängigkeit wird nicht dem Compiler überlassen, sondern ist eine explizit abgesicherte Eigenschaft der Architektur.

