# KLASSIK – Projektkontext für Claude Code

Eigene Firmware für den **M-VAVE FM-1**: mehrere klassische Synth-Engines in einem „Rack“, mit einer eigenständigen,
übersichtlichen Oberfläche. Jeder Sound hat eine **Glyphe** (siehe UI-Designsprache). Repo: github.com/fabiomorena/fm1-klassik.

Hardware: JieLi AC791N, 240×240-Farbdisplay, 27 Tasten, 14 Taster mit LEDs, 7 Encoder (SELECT, ALGO, PRESET, K1–K4)
plus MASTER, 1 MB Flash. **Fabio hat noch kein Gerät**: getestet wird nur im Host-Simulator, im Browser-Emulator und in der CI.
Nie behaupten, etwas sei auf Hardware getestet.

## Zusammenarbeit mit Fabio
- Antworten auf Deutsch. Code, Code-Kommentare und Bezeichner auf Englisch (Stil wie in `base/`).
- Erst einen kurzen Plan zeigen, dann umsetzen. Befehle für Fabio einzeln und mit erwarteter Ausgabe.
- Vor `git commit` und `git push` immer fragen.
- Fakten mit Quelle (Datei, Doku-Link), Unsicheres ausdrücklich als unsicher kennzeichnen, keine erfundenen Daten.
- Rechnungen (Speicher, Pegel, Zeiten) mit Rechenweg zeigen.
- Neue Erkenntnisse mit Quelle in `docs/RESEARCH.md`, neue Entscheidungen mit Datum in `docs/DECISIONS.md` eintragen.
- Design-Änderungen zuerst im passenden Mockup in `docs/design/` (Fabio sieht sie über `docs/design/index.html`), dann im Code.

## Aufbau
| Pfad | Was |
| --- | --- |
| `base/` | fm1-base (Submodul, MPL-2.0): Plattform, Host-Simulator, Browser-Emulator, `tools/build.py`. **Nicht ändern.** |
| `engines/` | Felucca-Engines (Submodul, MIT): ANALOG, PHASE, WHEEL, LOFI, TRIO, NOISE. **Nicht ändern.** |
| `firmware/klassik/app.c`, `app.h` | die App (Unity-Build nach der Plattform): UI, Eingabe, Einstellungen, Audio-Ausgabe |
| `firmware/klassik/dsp/engines.c` | Stimmen-Host (o2-Einheit, MIT, nach `engines/tests/fe_host.c`): 1 Part, 6 Stimmen, eigene einfache ADSR |
| `firmware/klassik/dsp/klassik_dsp.h` | Schnittstelle App ↔ DSP |
| `firmware/klassik/fm1.toml` | Build-Einstellungen (o2, include, gen für `fe_tables.h`) |
| `firmware/klassik/tests/*.txt` | Host-Tests (Syntax: Kopfkommentar von `base/host/plat_host.c`; eigene Prüfungen über `app_expect`) |
| `docs/CONCEPT.md` | Konzept und Fahrplan |
| `docs/design/` | Mockups aller Screens (HTML + PNG, Übersicht `index.html`, siehe `docs/design/README.md`), **maßgebliche Design-Quelle** |
| `docs/RESEARCH.md` | Recherche mit Quellen: Hardware, Firmwares, Engines, Lizenzen, Entwicklungsumgebung |
| `docs/DECISIONS.md` | Entscheidungen mit Datum und Status, offene Punkte |

## Bauen und testen (Mac, im Repo-Stamm)
```
export DYLD_FALLBACK_LIBRARY_PATH=/opt/homebrew/lib
.venv/bin/python base/tools/build.py firmware/klassik --host
for t in firmware/klassik/tests/*.txt; do firmware/klassik/build/host/klassik_host "$t" firmware/klassik/build/host || break; done
```
- Die Variable braucht Pillow, um libfribidi zu finden (libraqm für die Schriften); sie steht auch in `~/.zshrc`.
- Nach jeder Änderung: bauen, alle Tests laufen lassen (jede Zeile „ok“, Exit-Code 0). Bei UI-Änderungen die PNGs in
  `firmware/klassik/build/host/` ansehen und mit `docs/design/png/` vergleichen.
- Die CI (`.github/workflows/host.yml`) macht dasselbe bei jedem Push.
- Geräte-Build (`base/tools/build.py firmware/klassik`) braucht Docker und die JieLi-Toolchain: noch nicht eingerichtet.

## Plattform-Regeln (fm1-base)
- Vor Plattform-Arbeit lesen: `base/README.md`, `base/DESIGN.md`, `base/include/plat.h`, `base/template/app.c`.
- `app.c` hat `gfx.c` im Scope: `cv_begin`, `cv_rect`, `cv_line`, `cv_text`, `text_w`, `cv_blit`, `lcd_fill`, `RGB()`,
  Fonts `FONT_T` (10 px), `FONT_XS`, `FONT_S`, `FONT_B`, `FONT_M`, `FONT_L` (M und L nur Großbuchstaben).
  Eine Zeichenfläche ist höchstens 240×144: in Streifen zeichnen.
- Budget (base/README.md): 581 KB App-Slot, 96 KB RAM für .data/.bss, 336 KB Pool.
- Den Update-Weg der Plattform nie entfernen oder umgehen (sonst ist ein Gerät ohne Rückweg gesperrt).
- Änderungen nur in `firmware/klassik/`, nie in `base/` oder `engines/`.

## Klang (Stand v0)
- Engines: VA = ANALOG, PD = PHASE, ORGAN = WHEEL. Je Engine acht Parameter (`engines/src/felucca_engine.h`:
  `engine_t.edit[8]`, `page_title[2]`, `presets`). Festkomma, keine Allokation.
- Pegel: ×32 bei vollem MASTER; eine Stimme misst im Simulator −31,5 dBFS bei MASTER 50 % (Gain quadratisch:
  0,5² = 0,25 ≈ −12 dB), also etwa −19,5 dBFS bei vollem MASTER. Feinabstimmung erst mit Gerät.

## Lizenz-Regeln
- Eigene Dateien MPL-2.0, `dsp/engines.c` MIT. **Keine GPL-Quellen** übernehmen (z. B. Dexed, Felucca-App-Code,
  KR-106, Shine On). Erlaubt und geplant: Felucca-Engines (MIT), msfa (Apache-2.0) für FM6, Open303 (MIT) für ACID.
- Jede neue Quelle vor der Übernahme in `LICENSING.md` eintragen.
- `id_prefix = "FM-1_77"` ist noch nicht auf Kollisionen geprüft.

## UI-Designsprache
- Raster jedes Screens: Kopfzeile 22 px (Engine-Chip in Engine-Farbe, Glyphe, Soundname, Oktave, Akku) · Inhalt ·
  Fußzeile 32 px (was die Regler gerade halten, je Feld ein Balken).
- Farben (RGB): ground 11,13,17 · head 17,21,28 · surface 21,25,34 · surface2 28,34,48 · line 42,50,66 ·
  text 232,234,237 · muted 154,163,178.
  Engines: FM6 92,200,255 · VA 255,180,84 · PD 255,122,182 · ORGAN 126,224,138 · STRINGS 185,162,255 · ACID 227,244,94.
- **Glyphe:** die acht Parameter eines Sounds als acht Strahlen, im Uhrzeigersinn ab oben, Länge
  r × (0,25 + 0,75 × Wert/255) (Wert = Position im Bereich des Parameters, `kl_shape`), Fläche 3/8 Engine-Farbe über
  dem Hintergrund, Rand 2 px in Engine-Farbe, Mittelpunkt 3×3. Erscheint überall, wo ein Sound genannt wird.
  Maßgeblich ist `glyph_draw` in `app.c`; die Glyphen in den Mockups sind nur Beispiele.
- Kleinste Schrift auf dem Gerät: `FONT_T`; wo möglich `FONT_XS` oder größer.
- Bestätigen / Zurück auf Unterseiten: OCT+ / OCT− (wie in Felucca).
- Signatur-Ideen (Klang-Atlas, Glyphen-Browser, Klang-Stammbaum): `docs/CONCEPT.md`, Mockups 08–10.

## Fahrplan
| Version | Inhalt | Stand |
| --- | --- | --- |
| v0 | Rack, VA / PD / ORGAN, Glyphen, Speichern, Host-Tests, CI | erledigt (Simulator) |
| v0.1 | Edit-Seiten (EDIT → 2 Seiten × 4 Parameter auf K1–K4), Live-Glyphe, eigener Sound pro Engine (SAVE), README-Hinweis fribidi/.venv, Tests mit Screenshots | erledigt (Simulator) |
| v0.2 | FM6 (msfa, Apache-2.0) | offen |
| v0.3 | ACID (Open303, MIT), STRINGS (ANALOG + Ensemble-Chorus) | offen |
| v0.4 | Preset-Liste, Glyphen-Browser | offen |
| v0.5 | Sequencer, Mixer, Effekte | offen |
| v0.6 | Klang-Atlas, Klang-Stammbaum | offen |
