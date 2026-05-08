# AXIOM TRADER - Build Log

## Projekt-Status: v0.4.0-alpha
**Milestone:** Interaktive World-Engine & Layout-Stabilisierung  
**Datum:** Freitag, Abschluss Woche 1  

---

## 🛠 Tech-Stack & Umgebung
- **OS:** macOS 15.x (Apple Silicon Optimized)
- **Grafik-API:** Metal (CAMetalLayer Hosting)
- **UI-Framework:** Dear ImGui (v1.9x) + GLFW
- **Sprachstandard:** C++20 / Objective-C++
- **Build-System:** CMake (Xcode-Bundle Generation)

---

## ✅ Erreichte Meilensteine (Sprint v0.4.0)

### 1. Interaktives World-Monitoring
- **Geospatial Mapping:** Koordinaten-Datenbank für globale Handelsplätze (NYC, LDN, FRA, TYO, SYD).
- **Symbol-Interaction:** Hit-Detection auf der Karte (Pythagoras-Distanz-Check). Klicks laden Daten direkt in die Master-Eingabemaske (Load-to-Edit).
- **Directional Graphics:** Visuelle Unterscheidung: LONG (▲) / SHORT (▼).
- **Asset-Hashing:** Automatisierte Farbgeneierung basierend auf dem Asset-Symbol für konsistente Optik.

### 2. Layout & UX Stabilität
- **Window Management:** Layout-Engine mit Reset-Funktion zur Wiederherstellung des Standard-Grids (Master-Control & World-Monitor).
- **Retina Fixes:** Synchronisation von Framebuffer-Skalierung und View-Bounds gegen UI-Einfrieren und Unschärfe.

### 3. Metal Pipeline Optimierung
- **Color-Attachment Fix:** Stabilisierung der Render-Pass-Deskriptoren durch explizite Index-Adressierung (`colorAttachments[0]`).

---

## 🐛 Gelöste Fehler (Hotfixes)
- **[BUG] ID-Conflict:** Fix für identische Asset-Namen via `ImGui::PushID(it->id)`.
- **[BUG] Event-Occlusion:** Fix der Spalten-Logik; `DEL`-Buttons sind nun trotz Selektion klickbar.
- **[BUG] Memory:** Korrektes Array-Handling für `snprintf` zur Vermeidung von Buffer-Overflows.

---

## 📅 Roadmap (Vorschau v0.5.x)
- [ ] **Infrastructure:** SQLite-Integration für deterministische Trade-Persistenz.
- [ ] **Visuals:** Integration der `stb_image.h` zum Laden einer PNG-Weltkarte.
- [ ] **Analytics:** Implementierung einer globalen PnL-Summen-Anzeige.

---

### **Repo-Status:** 
`Checkpoint v0.4.0-alpha - Build Stable`
