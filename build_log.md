# AXIOM TRADER - Project Build Log

## DATEI-INFO
*   **Projektname:** AXIOM Trader
*   **Version:** v35.0 (Consolidated Interactive Release)
*   **Plattform:** macOS 15.x (Apple Silicon / Intel)
*   **Tech-Stack:** C++, Dear ImGui, Metal, GLFW, Objective-C++ (Cocoa)

---

## AKTUELLE MEILENSTEINE (Stand Build v35.0)

### 1. Metal & Cocoa Stabilitäts-Kern
*   **Layer-Hosting Fix:** Umstellung auf ein echtes `Layer-Hosting` Modell. Die Reihenfolge der Initialisierung wurde als kritisch identifiziert:
    1. `[view setLayer:layer]`
    2. `[view setWantsLayer:YES]`
*   **Retina Synchronization:** Implementierung einer robusten Synchronisation zwischen `view.bounds` (Punkte) und `layer.drawableSize` (Pixel) unter Einbeziehung des `backingScaleFactor`. Behebt den "Black Screen" Fehler bei Fenster-Resizing.

### 2. Geometry & World Engine
*   **Ear-Clipping Engine:** Eigene Triangulations-Logik zur Füllung konkaver Polygone (z.B. Eurasien-Kontinent).
*   **Normalization Pipeline:** Automatisches Entfernen von redundanten Endpunkten (first == last) und Erzwingen der CCW-Winding-Order zur Vermeidung von Artefakten ("Knubbeln") an Nahtstellen.
*   **Aspect Ratio Correction:** Implementierung einer Letterbox-Logik, die die Weltkarte starr im 2:1 Format hält, unabhängig von der Fenstergröße des Widgets.

### 3. SYNAPSE Master-Control
*   **Interactive Editing:** Implementierung eines State-Systems. Das Anklicken eines Eintrags in der Tabelle lädt die Daten (Asset, Preise) zurück in die Master-Eingabemaske für Updates.
*   **Chrono-Mapping:** Verknüpfung der Trades mit der Weltkarte basierend auf UTC-Zeitstempeln.

---

## GELÖSTE KRITISCHE BUGS
*   **[BUG] Black Screen after First Frame:** Gelöst durch expliziten Zugriff auf `rp.colorAttachments[0].texture` und korrekte Hosting-Reihenfolge in Cocoa.
*   **[BUG] Stonehenge Map:** Abstrakt-geometrische Landmassen durch hochaufgelöste Vektor-Pfade und Triangulation ersetzt.
*   **[BUG] Map Distortion:** Behoben durch manuelle Berechnung des Proportional-Canvas innerhalb der `RenderAxiomWorld`.
*   **[BUG] Metal Selector Error:** Behoben durch explizite Typisierung des `MTLRenderCommandEncoder` im Objective-C++ Kontext.

---

## BACKLOG / NÄCHSTE SCHRITTE
- [ ] **Modul: Markt-Sessions:** London/NY/Tokio Glow-Zonen auf Weltkarte.
- [ ] **Modul: Persistence:** SQLite-Anbindung zur Trade-Speicherung.
- [ ] **Chrono-X Charts:** Asset-Fenster mit zeitlich korrekt skalierten X-Achsen.
