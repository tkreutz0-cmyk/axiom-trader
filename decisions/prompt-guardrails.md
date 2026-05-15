# KI-Code-Generierung: System-Leitplanken (Prompt Guardrails)

Dieses Dokument definiert die unumstößlichen Regeln für sämtliche KI-generierten Code-Beiträge im *AXIOM Trader*. Jede Code-Generierung (z.B. via GitHub Copilot, Claude oder ChatGPT) MUSS gegen diese Richtlinien validiert werden.

---

## 1. Multi-Plattform & Portabilität (Das Blender-Prinzip)
* **Core-Isolation:** Der Code im Verzeichnis `src/core/` muss zu 100 % aus plattformunabhängigem C++20 bestehen.
* **Header-Verbot:** In `src/core/` dürfen KEINE macOS-spezifischen Header (Cocoa, Metal, Objective-C++) oder Windows-spezifischen Header (Win32, DirectX) inkludiert werden.
* **UI-Entkopplung:** Die grafische Oberfläche (Dear ImGui) wird strikt in den plattformspezifischen Host-Layern (`src/macOS/`, `src/Windows/`) gekapselt.

## 2. Mathematische Integrität & Daten-Governance (ADR-0004)
* **Absolutes Float-Verbot:** Für Geldbeträge, Asset-Preise, Transaktionswerte und Kontoguthaben dürfen NIEMALS die Datentypen `float` oder `double` verwendet werden.
* **Fixed-Point-Zwang:** Alle monetären Berechnungen müssen ausnahmslos über die Struktur `FixedPoint` (`src/core/utils/fixed_point.hpp`) abgewickelt werden.
* **Ganzzahl-Basis:** Intern operiert das System mit `int64_t` auf Basis von Micro-Cents (4 feste Dezimalstellen, Skalierungsfaktor 10000).

## 3. High-Performance & Speicher-Management (Hotpath-Regeln)
* **Zero-Allocation im Hotpath:** Innerhalb der zentralen Handelsschleife (Trading Loop) und der Metal/Vulkan-Renderschleife sind Heap-Allokationen (`new`, `malloc`, `std::make_shared`, dynamische `std::vector`-Vergrößerungen) STRENGSTENS VERBOTEN.
* **Ressourcen-Sicherheit:** Außerhalb des Hotpaths ist die Speicherverwaltung ausnahmslos über RAII (`std::unique_ptr`, Stack-Allokation) abzusichern. Rohe Pointer dürfen nur lesend/beobachtend ohne Besitzrechte genutzt werden.
* **Compiler-Attribute:** Jede wertrückgebende Funktion im Core muss mit dem Attribut `[[nodiscard]]` versehen werden, um ungenutzte Rückgabewerte und logische Fehler beim Kompilieren zu verhindern.

## 4. Validierung im Drei-Instanzen-Modell
* Generierter Code gilt standardmäßig als *„untrusted“* (Instanz 1).
* Jedes Refactoring muss zwingend durch automatisierte Invarianten-Prüfungen (Instanz 3, z.B. Clang-Tidy, statische Assertions, Unit-Tests) falsifiziert werden, bevor ein Merge in den Hauptzweig erfolgt.
