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

---

## ✅ Erreichte Meilensteine (Sprint v0.4.1 — Refactor & Build Stabilisierung)

### 5. Deterministischer Core‑Refactor (neu)
- **Domain‑Extraktion:** Auflösung des bisherigen App‑Monolithen in eine eigenständige `Axiom::Trade`‑Klasse.
  - Saubere Trennung von Preislogik (`priceDelta`), FX‑Metadaten (`isFX`, `isJPY`) und PnL‑Berechnung.
  - Entfernung aller UI‑Abhängigkeiten aus der Core‑Logik (headless, test‑fähig).
- **Metadata‑Caching:** FX‑Erkennung & JPY‑Sonderlogik werden nur bei Symbol‑Änderung berechnet (kein Per‑Frame‑Parsing).
- **API‑Konsolidierung:** Vereinheitlichung kritischer Schnittstellen (`setSymbol(std::string_view)`) zur Vermeidung von Overload‑Ambiguitäten.

### 6. PnL‑Architektur‑Aufspaltung (neu)
- **Policy‑Layer (`Pnl.hpp`):**
  - Zentrale Umschaltlogik für *Price‑Delta*, *Pips* und *Money*.
  - Keine eigene Berechnung, ausschließlich Delegation an den Trade‑Core.
- **Formatter‑Layer (`PnlFormatter.hpp`):**
  - String‑Formatierung als reine UI‑Nähe‑Schicht.
  - Adaptive Präzision abhängig von FX / Non‑FX.
- **Single‑Source‑of‑Truth:** PnL‑Berechnungen existieren ausschließlich im Trade‑Core.

### 7. World‑Subsystem Modularisierung (neu)
- **WorldModel:** Extraktion der Cluster‑ und Aggregationslogik in ein headless Datenmodell.
  - Symbol‑basierte Clusterbildung mit LONG/SHORT‑Statistik.
  - Entkopplung von Rendering und Datenaggregation.
- **WorldUiState:** Expliziter UI‑State für Interaktionen (Cluster‑Popup, Fokus, Anchor).
  - Keine statischen Variablen mehr.
  - Deterministischer, debuggbarer Interaktionszustand.
- **WorldRenderer Interface:** Klare Trennlinie zwischen Daten, State und Drawing (Vorbereitung für `.cpp`‑Implementierung).

### 8. Build‑Pipeline Konsolidierung (CMake / Xcode) (neu)
- **Include‑Root‑Fix:** Vereinheitlichung der Include‑Pfadlogik (`${CMAKE_CURRENT_SOURCE_DIR}` als Root).
- **Target‑Sanity:** Explizites Registrieren aller `.cpp`‑Dateien im CMake‑Target.
- **Linker‑Stabilisierung:** Behebung mehrerer ODR‑ und Symbol‑Mismatches durch strikt synchronisierte Header/CPP‑Signaturen.
- **ODR‑Safety:** Vollständige Entfernung von `.cpp`‑Includes.
- **Clean‑Build‑Regime:** Verlässliche Rebuilds nach strukturellen Änderungen (`rm -rf build`).

### 9. Core‑Test‑Binary (Status)
- **Headless Test‑Main:** Temporäres `main` zur Validierung der Core‑Logik.
- **Runtime‑Status:** Programm startet und beendet sich korrekt mit `Exit Code 0`.
- **Architektur‑Entscheidung:** Assertions verbleiben im Test‑Binary, nicht im GUI‑Target.

---

## 🐛 Gelöste Fehler (Refactor‑bedingte Fixes)

- **[BUILD] Header/CPP‑Mismatch:** Korrektur nicht übereinstimmender Funktionssignaturen (`setSymbol`, `refreshMetadata`).
- **[BUILD] Overload‑Ambiguität:** Eliminierung konkurrierender `setSymbol`‑Overloads.
- **[BUILD] Missing Translation Unit:** Fix für nicht gelinkte Core‑Implementationen (`Trade.cpp`).
- **[BUILD] Include‑Pfadfehler:** Auflösung fehlerhafter relativer Includes („one directory too deep“).
- **[RUNTIME] Immediate Exit:** Erwartetes Verhalten des Core‑Test‑Binary dokumentiert (kein UI‑Loop).

---

## 🧭 Aktueller Architektur‑Status

- ✅ Deterministischer Core isoliert & stabil
- ✅ World‑Subsystem sauber modularisiert
- ✅ Build reproduzierbar & konfliktfrei
- ✅ Grundlage für Re‑Integration des GUI‑Main‑Loops geschaffen

**Nächster geplanter Schritt:**  
Trennung in zwei Targets (`AxiomCoreTest` ↔ `AxiomTrader.app`) und Wiederanbindung des ImGui/Metal‑Event‑Loops.
