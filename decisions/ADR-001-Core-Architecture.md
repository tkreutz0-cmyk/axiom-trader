# ADR-001: Wahl des Technology-Stacks und Distributionsmodells

**Status:** Decided  
**Datum:** 2026-05-04  
**Beteiligte:** tkreutz

## Kontext
Es soll ein Trading-Journal entwickelt werden, das maximale Performance (C++20) mit voller grafischer Freiheit verbindet. Die App muss unabhängig von OS-Releases und App-Stores funktionieren.

## Entscheidung
1. **Core:** C++20 (SYNAPSE-Kern).
2. **UI:** Dear ImGui mit Metal-Backend (macOS).
3. **Distribution:** Standalone Desktop-Binary (Blender-Modell).
4. **Build-System:** CMake mit Xcode-Generator.

## Konsequenzen
- Hohe Portabilität (Windows/Linux möglich).
- Keine Abhängigkeit von SwiftUI für die Kern-Funktionalität.
- Manuelle Implementierung der Chart-Komponente notwendig.
