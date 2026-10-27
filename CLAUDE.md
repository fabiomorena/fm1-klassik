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
| `firmware/klassik/dsp/engines.c` | Stimmen-Host (o2-Einheit, MIT, nach `engines/tests/fe_host.c`): 1 Part, 6 Stimmen, eigene einfache ADSR; FM6, DRUMS und ACID mit eigenen Hüllkurven (`ownenv`, `done`); DRUMS: eine Stimme je Instrument (`same_note`); MONO-Presets (ACID): gehaltene Tasten (`held`), `legato` 1 = Slide, 2 = zurück zur gehaltenen Taste |
| `firmware/klassik/dsp/eng_fm6.c` | FM6 als `engine_t` (MPL-2.0): acht Makros, eigene Presets (DX7-Stimmen) |
| `firmware/klassik/dsp/fm6/` | msfa nach C portiert (o2-Einheit, Apache-2.0, `LICENSE`); Änderungen im Dateikopf von `msfa.c` |
| `firmware/klassik/tools/gen_fm6_tables.py` | rechnet msfas Tabellen beim Build (`gen` → `fm6_tables.h`, Apache-2.0) |
| `firmware/klassik/dsp/eng_drums.c` | DRUMS als `engine_t` (MPL-2.0): vier Kits, Tastenbelegung je Halbton (G# je Kit), Toms/Clap/Rimshot/Cowbell, Choke, Accent |
| `firmware/klassik/dsp/drums/` | Plaits-Drums nach C portiert (o2-Einheit, MIT, `float`): 808-BD/SD/Hi-Hat, 909-artige BD/SD; Änderungen im Dateikopf von `drums.c` |
| `firmware/klassik/dsp/eng_acid.c` | ACID als `engine_t` (MPL-2.0): acht Makros in den Einheiten von Open303, acht Presets, Accent ab Velocity 100 |
| `firmware/klassik/dsp/acid/` | Open303 nach C portiert (o2-Einheit, MIT, `float`, TB_303-Filter); Änderungen im Dateikopf von `acid.c` |
| `firmware/klassik/tools/gen_acid_tables.py` | rechnet Open303s Wellenform-Tabellen (int16), das elliptische Filter als Biquads und feste Koeffizienten beim Build (`acid_tables.h`, MIT) |
| `firmware/klassik/dsp/klassik_dsp.h` | Schnittstelle App ↔ DSP |
| `firmware/klassik/fm1.toml` | Build-Einstellungen (o2: `engines.c`, `msfa.c`, `drums.c`, `acid.c`; include; gen für `fe_tables.h`, `fm6_tables.h` und `acid_tables.h`) |
| `firmware/klassik/tests/*.txt` | Host-Tests (Syntax: Kopfkommentar von `base/host/plat_host.c`; eigene Prüfungen über `app_expect`) |
| `docs/CONCEPT.md` | Konzept und Fahrplan |
| `docs/design/` | Mockups aller Screens (HTML + PNG, Übersicht `index.html`, siehe `docs/design/README.md`), **maßgebliche Design-Quelle** |
| `docs/RESEARCH.md` | Recherche mit Quellen: Hardware, Firmwares, Engines, Lizenzen, Entwicklungsumgebung |
| `docs/DECISIONS.md` | Entscheidungen mit Datum und Status, offene Punkte |
| `docs/tests/` | Testprotokolle zum Abhaken (Browser-Emulator): Fabio füllt sie aus, Claude wertet sie am Ende der Datei aus |

## Bauen und testen (Mac, im Repo-Stamm)
```
export DYLD_FALLBACK_LIBRARY_PATH=/opt/homebrew/lib
.venv/bin/python base/tools/build.py firmware/klassik --host
for t in firmware/klassik/tests/*.txt; do firmware/klassik/build/host/klassik_host "$t" firmware/klassik/build/host || break; done
```
- Die Variable braucht Pillow, um libfribidi zu finden (libraqm für die Schriften); sie steht auch in `~/.zshrc`.
- Nach jeder Änderung: bauen, alle Tests laufen lassen (jede Zeile „ok“, Exit-Code 0). Bei UI-Änderungen die PNGs in
  `firmware/klassik/build/host/` ansehen und mit `docs/design/png/` vergleichen.
- Die CI macht dasselbe bei jedem Push (`.github/workflows/host.yml`) und baut den Browser-Emulator
  (`.github/workflows/web.yml`, Emscripten 6.0.13 aus emsdk). Lokal: `base/tools/build.py firmware/klassik --web`
  (Emscripten per Homebrew), ansehen mit `python3 -m http.server 8000 -d firmware/klassik/build/emu`.
- Geräte-Build (`base/tools/build.py firmware/klassik`) braucht Docker und die JieLi-Toolchain: noch nicht eingerichtet.

## Plattform-Regeln (fm1-base)
- Vor Plattform-Arbeit lesen: `base/README.md`, `base/DESIGN.md`, `base/include/plat.h`, `base/template/app.c`.
- `app.c` hat `gfx.c` im Scope: `cv_begin`, `cv_rect`, `cv_line`, `cv_text`, `text_w`, `cv_blit`, `lcd_fill`, `RGB()`,
  Fonts `FONT_T` (10 px), `FONT_XS`, `FONT_S`, `FONT_B`, `FONT_M`, `FONT_L` (M und L nur Großbuchstaben).
  Eine Zeichenfläche ist höchstens 240×144: in Streifen zeichnen.
- Budget (base/README.md): 581 KB App-Slot, 96 KB RAM für .data/.bss, 336 KB Pool.
- Den Update-Weg der Plattform nie entfernen oder umgehen (sonst ist ein Gerät ohne Rückweg gesperrt).
- Keine 64-Bit-Multiplikation inline in DSP-Schleifen: Neben Ladebefehlen rechnet der FM-1 dann gelegentlich falsch
  (`base/bsp/docs/CPU.md`). Stattdessen 32 Bit mit Rechenweg, oder eine `noinline`-Hilfsfunktion. Kein `double`.
- Änderungen nur in `firmware/klassik/`, nie in `base/` oder `engines/`.

## Klang (Stand v0.3)
- Engines: VA = ANALOG, PD = PHASE, ORGAN = WHEEL, FM6 = msfa (`KL_FM6`, im Rack links von VA; K1 folgt der
  Rack-Reihenfolge), DRUMS (`KL_DRUMS` = 4, Kachel der früheren STRINGS). Je Engine acht Parameter (`engines/src/felucca_engine.h`:
  `engine_t.edit[8]`, `page_title[2]`, `presets`). Festkomma, keine Allokation.
- FM6: Presets sind ganze DX7-Stimmen plus acht beschreibende Werte; die Makros verschieben die Stimme relativ zum
  Preset (Regel im Kopf von `eng_fm6.c`). ALGO/FEEDBK live, die anderen ab der nächsten Note. Der Port ist gegen
  das Original-msfa (portabler Pfad, nicht NEON) geprüft, siehe `docs/RESEARCH.md`.
- DRUMS: Kits „606“, „707“, „808“, „909“ (Presets); Taste je Halbton → Instrument (`DR_OF_PC`; G# `DR_X` je Kit:
  RD, TB, CL, RD); die Oktave ändert nichts; die acht Makros setzen, was sie nennen (BD/SD/TOM TUNE in Halbtönen,
  HAT DEC/TONE für CH und OH, HAT TONE 1,9–9,6 kHz). Pegel je Modell über `DM_NORM` eingemessen
  (BD ~−24, SD ~−26, Hats ~−29 dBFS bei MASTER 50 %). Port gegen Plaits geprüft (RESEARCH.md). Edit-Seite: Pads statt
  Glyphe (`draw_pads`).
- ACID (`KL_ACID` = 5, letzte Kachel): Open303 mit TB_303-Filter, eine Stimme. Slide = Legato, Accent = ENV beim
  Anschlag gehalten (die App gibt dann Velocity 127, sonst 64; MIDI: ab 100), `ACC` in der Kopfzeile. Makros TUNE,
  CUTOFF, RESO, ENV MOD / DECAY, ACCENT, WAVE, SLIDE; DECAY und ACCENT ab der nächsten Note. Pegel `AC_LEVEL`
  2 × VOICE_FS: normale Noten −27 … −33, Accents −16 … −25 dBFS bei MASTER 50 %; hohe Lagen mit Resonanz und
  Accent übersteuern bei vollem MASTER. Port gegen Open303 geprüft (56–67 dB, RESEARCH.md).
- Pegel: ×32 bei vollem MASTER; eine Stimme misst im Simulator −31,5 dBFS bei MASTER 50 % (Gain quadratisch:
  0,5² = 0,25 ≈ −12 dB), also etwa −19,5 dBFS bei vollem MASTER. Feinabstimmung erst mit Gerät.

## Lizenz-Regeln
- Eigene Dateien MPL-2.0, `dsp/engines.c`, `dsp/drums/`, `dsp/acid/` und `tools/gen_acid_tables.py` MIT, `dsp/fm6/` und
  `tools/gen_fm6_tables.py` Apache-2.0.
  **Keine GPL-Quellen** übernehmen (z. B. Dexed, Felucca-App-Code, KR-106, Shine On). Übernommen: Felucca-Engines (MIT), msfa (Apache-2.0) für FM6, Plaits-Drums (MIT) für DRUMS,
  Open303 (MIT, aus dem Original-Repo) für ACID. Keine Samples aus Rolands Geräten.
- Jede neue Quelle vor der Übernahme in `LICENSING.md` eintragen.
- `id_prefix = "FM-1_77"` ist noch nicht auf Kollisionen geprüft.

## UI-Designsprache
- Raster jedes Screens: Kopfzeile 22 px (Engine-Chip in Engine-Farbe, Glyphe, Soundname, Oktave, Akku) · Inhalt ·
  Fußzeile 32 px (was die Regler gerade halten, je Feld ein Balken).
- Farben (RGB): ground 11,13,17 · head 17,21,28 · surface 21,25,34 · surface2 28,34,48 · line 42,50,66 ·
  text 232,234,237 · muted 154,163,178.
  Engines: FM6 92,200,255 · VA 255,180,84 · PD 255,122,182 · ORGAN 126,224,138 · DRUMS 185,162,255 · ACID 227,244,94.
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
| v0.2 | FM6 (msfa, Apache-2.0, nach C portiert), acht eigene Presets | erledigt (Simulator) |
| v0.3 | DRUMS (Kits 606, 707, 808, 909; Plaits, MIT; ersetzt STRINGS), dann ACID (Open303, MIT) | erledigt (Simulator) |
| v0.4 | Preset-Liste, Glyphen-Browser | offen |
| v0.5 | Sequencer, Mixer, Effekte | offen |
| v0.6 | Klang-Atlas, Klang-Stammbaum | offen |
