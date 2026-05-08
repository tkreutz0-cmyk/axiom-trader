# AXIOM Trader - Technisches Logbuch

## Projekt-Status: v0.1-alpha "Base Canvas"

### System-Architektur
Das System basiert auf einem deterministischen Ansatz (Blender-Modell) und nutzt ein natives macOS-Backend für maximale Performance bei der Datenvisualisierung.

- **Frontend:** Dear ImGui mit Metal-Backend.
- **Windowing:** GLFW (DPI-aware für Retina-Displays).
- **Interaktions-Modell:** Optimiert für MacBook Pro (Trackpad-gesteuert).

### Aktueller Build-Stand
- [x] Metal-Renderer Initialisierung
- [x] Retina-Auflösung Support (Framebuffer-Skalierung)
- [x] Modul 1: SYNAPSE-Journal (Eingabemaske für Real-Daten)
- [x] Modul 2: Needle-Dashboard (Dynamisches Canvas mit Pan & Zoom)
- [x] Callback-Handling via `ImGui_ImplGlfw_InstallCallbacks`

### Steuerung (MacBook Pro)

| Aktion | Geste |
| :--- | :--- |
| **Verschieben (Pan)** | Zwei-Finger-Scroll auf dem Dashboard |
| **Zoomen** | `Option` (Alt) + Zwei-Finger-Scroll |
| **Fenster bewegen** | Klick & Drag auf die Titelleiste |
| **Eingabe** | Direkte Tastatureingabe im Journal-Fenster |

### Nächste Meilensteine
1. **Y-Achsen-Transformation:** Mapping von Welt-Koordinaten (Preis-Level) auf Bildschirm-Pixel.
2. **Persistence Layer:** Re-Integration der SQLite-Engine (Modul 2.3).
3. **SYNAPSE-Core:** Implementierung der mathematischen P&L-Berechnung in C++.

---
*Letzter Commit-Stand: Core-UI & Dashboard stabil.*
