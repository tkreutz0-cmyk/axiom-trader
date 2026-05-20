# AXIOM-Trader

> Deterministic Trading Journal Engine (C++20 / ImGui / Metal)

---

## Overview

AXIOM-Trader ist kein klassisches Trading-Journal.

Es ist eine deterministische Simulationsplattform, die sicherstellt,
dass nur gültige Systemzustände entstehen – nicht nur angezeigt werden.

"Nicht was passieren könnte — sondern was erlaubt ist."

---

## Core Prinzipien

- Determinismus über das gesamte System
- Strict Separation: Core vs UI
- Zero-Overhead im Hotpath
- Architektur > Features

---

## Architektur (End-to-End Pipeline)

SQLite → Domain Kernel → Mapping → UI → Rendering

### Pipeline Status

- Daten geladen ✅
- Core verarbeitet ✅
- UI dargestellt ✅
- Rendering sichtbar ✅

→ Pipeline vollständig geschlossen (kein Silent Failure mehr)

---

## Domain Kernel (C++20)

Der Core ist:

- vollständig plattformunabhängig
- frei von UI-Abhängigkeiten
- deterministisch

### Regeln

- kein ImGui
- kein Metal / OS Code
- keine Heuristiken
- nur reine Domänenlogik

---

## Deterministische Geldberechnung (ADR-0004)

Money Definition (Fixed Point):

    using Money = fixed_point<int64_t, 10000>;

### Regeln

- keine float oder double im Core
- nur int64-basierte Fixed-Point Arithmetik

### Vorteile

- keine Rundungsfehler
- vollständige Reproduzierbarkeit
- plattformunabhängig

---

## Persistence Layer (SQLite)

### Architektur

- DAO Pattern:
  - TradeDao
  - AssetSpecDao

### Features

- automatisches Schema (`ensureSchema()`)
- Lazy Statement Preparation
- deterministischer Start
- ausführliche Fehlerdiagnostik

---

## Mapping Layer (DB ↔ Core)

Trennung:

- DB: int64 raw values
- Core: Money (FixedPoint)

### Vorteile

- keine impliziten Konvertierungen
- keine Datenverluste
- klare Verantwortlichkeiten

---

## AXIOM World Engine (Stage D1)

### Features

- Weltkarte (Metal Texture)
- Trade-Marker (Long / Short)
- Clusterbildung
- deterministische Sortierung

---

## Rendering Pipeline (Critical Fix)

### Problem

- Daten vorhanden
- UI aktiv
- nichts sichtbar ("Silent Failure")

### Ursache

- fehlende Integration in Render Loop

### Lösung

- Integration von buildClusters()
- Fix der ImGui Render-Reihenfolge
- korrekte ImTextureID Nutzung

### Ergebnis

- Trades sichtbar ✅
- Pipeline geschlossen ✅
- stabil ✅

---

## UI-System

Dear ImGui:

- Immediate Mode GUI
- kein retained state
- deterministische Frame-Logik

---

## WorkspaceManager

- deterministisches Layout
- keine zufälligen Fensterpositionen
- reproduzierbare UI

### Grid

25% | 50% | 25%

---

## Rendering (Metal)

- native GPU Nutzung
- RAII Resource Handling (MapTexture)
- Bundle Resource korrekt integriert

---

## Tests & Validierung

- Pip-Berechnung
- PnL-Konsistenz
- Fixed-Point Genauigkeit

→ deterministisch validiert

---

## Architektur-Guardrails

Core Regeln:

- keine floats
- keine UI im Core
- deterministische Funktionen
- One Definition Rule enforced

---

## Roadmap

### Stage D2

- Async DB Worker
- Storage Command Queue
- Non-blocking UI

### Future

- Simulation Engine (SYNAPSE)
- Constraint-basierte Logik
- Governance Engine

---

## Architektur-Insight

AXIOM basiert nicht auf:

- Prognosen
- statistischen Modellen
- heuristischen Annahmen

sondern auf:

- deterministischen Regeln
- validierten Zuständen
- Architektur-Governance

---

## Fazit

AXIOM ist kein Tool.

Es ist eine deterministische Engine zur Erzwingung korrekter Systemzustände.
