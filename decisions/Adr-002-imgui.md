# ADR-002: Temporäre Nutzung von Homebrew für RAD-Prototyping

**Status:** Decided  
**Datum:** 2026-05-04  
**Beteiligte:** tkreutz

## Kontext
Um die Entwicklung des visuellen Dashboards (Nadel-Logik) zu beschleunigen und die aktuelle Hürde beim Setup der Git-Submodule zu umgehen, wird eine RAD-Umgebung (Rapid Application Development) benötigt.

## Entscheidung
Wir nutzen temporär **Homebrew** zur Bereitstellung der Abhängigkeiten (Dear ImGui), um eine sofort lauffähige Arbeitsumgebung auf dem MacBook Pro M5 zu erhalten. 

**Technische Details:**
- Installation via: `brew install imgui`
- Einbindung in CMake via Pfad-Referenz auf `/opt/homebrew/`
- IDE: Xcode (via CMake-Generator)

## Konsequenzen
- **Vorteil:** Sofortiger Start der UI-Entwicklung und grafischen Prototypisierung.
- **Nachteil:** Vorübergehende Verletzung der "Standalone-Philosophie" (Blender-Modell).
- **Refactoring-Schuld:** Vor dem ersten Release oder dem Portieren auf Windows/Linux müssen die Abhängigkeiten in den `external/` Ordner (Submodule) überführt werden.

## Nächste Schritte
1. Installation der Pakete via Homebrew.
2. Anpassung der `CMakeLists.txt` an die Homebrew-Pfade.
3. Generierung des Xcode-Projekts.
