# AXIOM TRADER - Project Build Log

## DATEI-INFO
*   **Projektname:** AXIOM Trader
*   **Version:** v0.4.0-alpha (Milestone: Metal & Geometry Core Stable)
*   **Plattform:** macOS 15.x (Apple Silicon / Intel)
*   **Tech-Stack:** C++20, Dear ImGui, Metal API, GLFW, Objective-C++ (Foundation)

---

## AKTUELLE MEILENSTEINE (Stand Build v0.4.0)

### 1. Metal & Cocoa Stabilitäts-Kern
*   **Layer-Hosting Fix:** Umstellung auf ein explizites `Layer-Hosting` Modell. Die Initialisierungsreihenfolge wurde stabilisiert:
    1. `[view setLayer:layer]`
    2. `[view setWantsLayer:YES]`
*   **Retina Synchronization:** Dynamische Koppelung von `view.bounds` und `layer.drawableSize` unter Berücksichtigung des `backingScaleFactor`. Beseitigung von Unschärfe und Black-Screen-Artefakten.

### 2. Geometry & World Engine
*   **Ear-Clipping Engine:** Implementierung eines robusten Ear-Clipping-Algorithmus zur fehlerfreien Triangulation konkaver Polygone (Eurasien/Afrika).
*   **Normalization Pipeline:** Automatisierte Säuberung der Vertex-Daten (Entfernung redundanter Endpunkte, Erzwingen der CCW-Winding-Order). Behebt Linien-Artefakte ("Knubbel") an Nahtstellen.
*   **Aspect Ratio Guard:** Proportionale 2:1 Skalierung der Weltkarte mit Letterboxing innerhalb des Widget-Containers.

### 3. SYNAPSE Master-Control
*   **Interactive State Engine:** Bidirektionale Verknüpfung von Tabelle und Editor. Selektion eines Eintrags lädt den Status direkt in die Master-Eingabemaske (Load-to-Edit).
*   **UTC Chrono-Mapping:** Verknüpfung der Trades mit geographischen Ankern basierend auf zeitgestempelten UTC-Daten.

---

## GELÖSTE KRITISCHE BUGS
*   **[RENDER] Black Screen:** Behoben durch explizite Adressierung von `rp.colorAttachments[0].texture` und korrektem Cocoa-Hosting.
*   **[GEOM] Stonehenge-Artifacts:** Abstrakte Umrisse durch High-Fidelity Vektorpfade mit aktiver Triangulation ersetzt.
*   **[UI] Focus Lock:** Fokus-Steuerung bei "GOTO"-Befehlen entkoppelt, um flüssiges Editieren zu ermöglichen.

---

## BACKLOG & ROADMAP

### Modul: Visual-Advanced (v0.4.x)
- [ ] **Market Session Zones:** Integration von LDN/NY/TYO Handelszeiten als semitransparente Glow-Bänder hinter der Weltkarte.
- [ ] **Day/Night Terminator:** Implementierung der mathematischen Schattenlinie basierend auf dem aktuellen Sonnenstand.
- [ ] **Chrono-X Needles:** Re-Integration der Asset-Charts mit zeitlich proportional skalierten X-Achsen.

### Modul: Infrastructure (v0.5.x)
- [ ] **Persistence Layer:** SQLite-Integration für deterministische Datenspeicherung in `~/Library/Application Support/AXIOM`.
- [ ] **Trade History CSV-Export:** Modul für den Datenexport zu Analysezwecken.

### Modul: SYNAPSE-Kernel (v0.6.x)
- [ ] **PnL Calculus Engine:** Umstellung der Gewinnberechnung auf Festkomma-Arithmetik (Precision Handling).
- [ ] **Multi-Currency-Support:** Basis-Währungs-Umrechnung für globale Asset-Paare.
