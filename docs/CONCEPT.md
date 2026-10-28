# KLASSIK – Konzept und Fahrplan

Mockups aller Screens (240×240, in 2× gezeichnet): [docs/design/](design/README.md), Übersicht `docs/design/index.html`.

## Leitidee

Ein Rack klassischer Synth-Engines. Jede Engine hat eine Signalfarbe; jeder Sound hat eine **Glyphe**, berechnet
aus seinen acht Parametern (acht Strahlen, Länge = Wert im Bereich des Parameters). Ähnliche Sounds sehen
ähnlich aus. Die Glyphe erscheint überall, wo ein Sound genannt wird: Kopfzeile, Rack, Preset-Liste, Sequencer,
Mixer.

Raster auf jedem Screen: Kopfzeile (Engine, Glyphe, Sound, Oktave, Akku) · Inhalt · Reglerzeile (was die Regler
gerade halten).

## Signatur-Ideen

- **Klang-Atlas:** alle Sounds aller Engines auf einer Karte (dunkel ↔ hell, weich ↔ scharf), mit zwei Reglern
  befahren; die Position je Sound wird beim Build aus Simulator-Renderings berechnet (2 Byte pro Sound).
- **Glyphen-Browser:** Sounds als Raster von Glyphen, sortiert nach Ähnlichkeit.
- **Klang-Stammbaum:** jede gespeicherte Variante ist ein Ast; Morph zwischen zwei beliebigen Ästen (A/B),
  MUTATE erzeugt neue Kinder (ca. 27 Byte pro Knoten).

## Fahrplan

| Version | Inhalt | Stand |
| --- | --- | --- |
| v0 | Rack, VA / PD / ORGAN, Glyphen, Speichern, Host-Tests | erledigt (Simulator) |
| v0.1 | Edit-Seiten je Engine (zwei Seiten à vier Regler), Live-Glyphe, USER-Sound je Engine (SAVE) | erledigt (Simulator) |
| mit Gerät | eigene Hüllkurve verfeinern, Pegel auf Hardware einmessen, Beschleunigung der Regler | offen |
| v0.2 | FM6 (msfa, Apache-2.0, nach C portiert), acht eigene Presets | erledigt (Simulator) |
| v0.3 | DRUMS (Kits im Stil von 606, 707, 808, 909; Plaits, MIT, und eigener Code; ersetzt STRINGS), ACID (Open303, MIT) | erledigt (Simulator) |
| v0.5 | Parts (4, je eine Engine), Mixer mit Limiter, Noten-Looper | erledigt (Simulator) |
| v0.6 | Step-Sequencer (auf den Ereignissen des Loopers), Effekte (Delay, Hall) | Step-Sequencer erledigt (Simulator), Effekte offen |
| v0.7 | Preset-Liste und Glyphen-Browser (früher v0.4) | offen |
| v0.8 | Klang-Atlas, Klang-Stammbaum (früher v0.6) | offen |
| am Ende | Feinschliff: Klang nach Gehör, Design-Fragen | offen |
