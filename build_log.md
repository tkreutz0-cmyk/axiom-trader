## ✅ Erreichte Meilensteine (Sprint v0.4.0)

### 1. Interaktives World-Monitoring
- **Geospatial Mapping:** Koordinaten-Datenbank für globale Handelsplätze (NYC, LDN, FRA, TYO, SYD).
- **Cluster Rendering (neu):**
  - Trades werden pro Asset **automatisch zu Clustern aggregiert**, sobald mehrere Trades denselben Standort teilen.
  - **Cluster-Visualisierung:** Kreis-Marker mit Count-Overlay (Trade-Anzahl).
  - **Hit-Detection:** Klick auf Cluster öffnet eine kontextuelle Auswahl-Liste aller enthaltenen Trades.
  - **Selektion:** Einzelne Trades können direkt aus dem Cluster-Popup ausgewählt und in den Edit-Modus geladen werden.
- **Tooltip-System:** Cluster zeigen beim Hover LONG/SHORT-Verteilung sowie Gesamtanzahl.
- **Directional Graphics:** Visuelle Unterscheidung: LONG (▲) / SHORT (▼).
- **Asset-Hashing:** Automatisierte Farbgenerierung basierend auf dem Asset-Symbol für konsistente Optik.
- **Clip-Safety:** Interaktionen werden strikt auf den Kartenbereich begrenzt (Input-Gating).

### 2. Trade- & PnL-Engine (erweitert)
- **Mehrdimensionale PnL-Darstellung (neu):**
  - *Preis-Delta* (absolute Kursdifferenz, FX-skalierungsfest).
  - *Pips* (FX-konform, inkl. JPY-Sonderlogik).
  - *Monetär* (Lot-/Unit-basiert, konfigurierbare Pip-Value-Heuristik).
- **Dynamischer Anzeige-Modus:** Umschaltbar zur Laufzeit über UI (ohne Trade-Rekonstruktion).
- **Non-FX-Fallback:** Unterstützung für Crypto/CFD über Unit-basierte Delta-Berechnung.
- **Numerische Präzision:** Adaptive Dezimalstellen verhindern Nullanzeigen bei kleinen FX-Werten.

### 3. Layout & UX Stabilität
- **Window Management:** Layout-Engine mit Reset-Funktion zur Wiederherstellung des Standard-Grids (Master-Control & World-Monitor).
- **State-Synchronisation:** Saubere Trennung zwischen selektiertem Trade, Cluster-Fokus und Edit-State.
- **Retina Fixes:** Synchronisation von Framebuffer-Skalierung und View-Bounds gegen UI-Einfrieren und Unschärfe.

### 4. Metal Pipeline Optimierung
- **Color-Attachment Fix:** Stabilisierung der Render-Pass-Deskriptoren durch explizite Index-Adressierung (`colorAttachments[0]`).
- **Draw-Order Konsistenz:** Deterministische Render-Reihenfolge für Cluster, Marker, Selection-Overlay und Tooltips.

---

## 🐛 Gelöste Fehler (Hotfixes)
- **[BUG] Trade-Overlap:** Marker-Überlagerungen bei identischen Asset-Positionen durch Cluster-System vollständig beseitigt.
- **[BUG] PnL 0.00 Anzeige:** FX-PnL wurde durch Rundung verschluckt → adaptive Skalierung & Pip-Modus eingeführt.
- **[BUG] ID-Conflict:** Fix für identische Asset-Namen via `ImGui::PushID(it->id)`.
- **[BUG] Event-Occlusion:** Fix der Spalten-Logik; `DEL`-Buttons sind nun trotz Selektion klickbar.
- **[BUG] Memory-Safety:** Korrektes Buffer-Handling via `snprintf` (Null-Terminierung garantiert).
