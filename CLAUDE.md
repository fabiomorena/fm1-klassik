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
| `firmware/klassik/app.c`, `app.h` | die App (Unity-Build nach der Plattform): UI, Eingabe, Einstellungen, Audio-Ausgabe, Mixer, Looper |
| `firmware/klassik/dsp/engines.c` | Stimmen-Host (o2-Einheit, MIT, nach `engines/tests/fe_host.c`): 6 Parts (`KL_NPART`, `prt[]`) zu je 6 Stimmen, gewichtete Obergrenze (`COST`, `KL_BUDGET`), eigene einfache ADSR; FM6, DRUMS und ACID mit eigenen Hüllkurven (`ownenv`, `done`); DRUMS: eine Stimme je Instrument (`same_note`); MONO-Presets (ACID): gehaltene Tasten (`held`), `legato` 1 = Slide, 2 = zurück zur gehaltenen Taste |
| `firmware/klassik/dsp/eng_fm6.c` | FM6 als `engine_t` (MPL-2.0): acht Makros, eigene Presets (DX7-Stimmen) |
| `firmware/klassik/dsp/fm6/` | msfa nach C portiert (o2-Einheit, Apache-2.0, `LICENSE`); Änderungen im Dateikopf von `msfa.c` |
| `firmware/klassik/tools/gen_fm6_tables.py` | rechnet msfas Tabellen beim Build (`gen` → `fm6_tables.h`, Apache-2.0) |
| `firmware/klassik/dsp/eng_drums.c` | DRUMS als `engine_t` (MPL-2.0): vier Kits, Tastenbelegung je Halbton (G# je Kit), Toms/Clap/Rimshot/Cowbell, Choke, Accent |
| `firmware/klassik/dsp/drums/` | Plaits-Drums nach C portiert (o2-Einheit, MIT, `float`): 808-BD/SD/Hi-Hat, 909-artige BD/SD; Änderungen im Dateikopf von `drums.c` |
| `firmware/klassik/dsp/eng_acid.c` | ACID als `engine_t` (MPL-2.0): acht Makros in den Einheiten von Open303, acht Presets, Accent ab Velocity 100 |
| `firmware/klassik/dsp/acid/` | Open303 nach C portiert (o2-Einheit, MIT, `float`, TB_303-Filter); Änderungen im Dateikopf von `acid.c` |
| `firmware/klassik/tools/device_check.py` | Geräte-Build bis zu den Prüfungen von fm1-base (Größe, RAM, Pool, kein double), ohne Paket (MPL-2.0) |
| `firmware/klassik/tools/gen_acid_tables.py` | rechnet Open303s Wellenform-Tabellen (int16), das elliptische Filter als Biquads und feste Koeffizienten beim Build (`acid_tables.h`, MIT) |
| `firmware/klassik/dsp/klassik_dsp.h` | Schnittstelle App ↔ DSP |
| `firmware/klassik/dsp/fx.c` | Send-Effekte (o2-Einheit, MPL-2.0): Ping-Pong-Delay (in float gerechnet, Leitungen 16 Bit), PRE vor dem Hall, Schlafen ohne Send; Puffer gibt die App (Pool) |
| `firmware/klassik/dsp/hall/` | Rings' Hall nach C portiert (o2-Einheit, MIT, `float`, `FxEngine<32768, 16 Bit>`); Änderungen im Dateikopf von `hall.c` |
| `firmware/klassik/fm1.toml` | Build-Einstellungen (o2: `engines.c`, `msfa.c`, `drums.c`, `acid.c`, `fx.c`, `hall.c`; include; gen für `fe_tables.h`, `fm6_tables.h` und `acid_tables.h`) |
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
- Geräte-Prüfung ohne Paket (Docker läuft, Toolchain in `~/.jieli/toolchain`):
  `JIELI_TOOLCHAIN=~/.jieli/toolchain .venv/bin/python firmware/klassik/tools/device_check.py`; alle Zeilen „ok“,
  Exit-Code 0. Zeigt App-Größe, RAM und Pool (Stand: RESEARCH.md „Geräte-Build“). Das Paket
  (`base/tools/build.py firmware/klassik`) braucht das AC79-SDK: noch nicht geladen (DECISIONS.md).
- Der FM-1 hat keine libc: in DSP-Einheiten nur `memset`, `memcpy`, `memcmp` (selbst deklarieren, wie `engines.c`).

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

## Klang (Stand v0.9)
- Parts: sechs gleichzeitig (am Anfang VA, DRUMS, ACID, FM6, PD, ORGAN: `PART_ENGINE`; das Rack (`TILES`) seit 29.10. in
  derselben Reihenfolge, Part 1–6 von links oben nach rechts unten; K1 wählt die Engines in dieser Reihenfolge), SEL (Taster) oder SELECT
  (Drehregler) wählt; Tasten, K1, PRESET, OCT, EDIT gehören dem gewählten (`set.part`, `part_cur()`, `eng_cur()` in
  `app.c`); MIDI-Kanal 1–6 = Part 1–6; Mixer drei Parts je Seite (die des gewählten), Looper, Step und FX alle sechs; `work`/`orig`/`edited`
  je Part und Engine. Obergrenze: alle Parts zusammen ≤ 240 Zehntel FM6-Stimmen.
- Mixer (GLO; GLO/OCT+ geht MIX → SEND → LIST → GLYPHS → MIX (`GT_*`), OCT− schließt): je Part LEVEL (Q11-Tabelle, −40 … +6 dB), PAN (Balance),
  MUTE, SOLO, `send[2]` in `part_set_t`; `mix_update()` setzt `mix_tgt`, `snd_tgt` (nach LEVEL/MUTE/SOLO, vor PAN,
  quadratisch) und `ret_tgt`; `app_render` gleitet, mischt stereo, Limiter (−0,5 dBFS) vor dem harten Begrenzen.
  Speicherformat `KLS9` (`KLS8` und `KLS7` mit vier Parts werden in `settings_load` übernommen).
- LFO (Taster LFO; LFO/OCT+ wechselt LFO/MORE; `set.mod[p]` in `app.c`, `lfo_*`; DSP `lfo_tick` in `dsp/engines.c`):
  je Part einer, RATE SYNC (Ticks, `LFO_SYNC`) oder Hz (`LFO_HZ`), SYNC + FREE folgt bei laufender Schleife
  `lp.pos16` (`lfo_loop_phase` im Render), DEPTH, WAVE, DEST (`kl_lfo_dests`: VA alle, PD PITCH/DCW/AMP, ORGAN und
  FM6 PITCH/AMP, ACID PITCH/FILTER, DRUMS keins), RETRIG, FADE. VA/PD/ORGAN über `vmod_t`, FM6 über
  `fm_note_compute(…, pitch_add)` und eine Verstärkungsrampe, ACID über `acid_set_pitch_mod`/`acid_set_cutoff`.
  Speicherformat `KLSA` (KLS9 wird übernommen).
- ARP (Taster ARP; auf der offenen Seite schaltet ARP LATCH, `arp_latch`, nicht gespeichert; `arp_*` in `app.c`):
  je Part einer, im Render (`arp_tick` je 32 Frames nach `lp_step`; Tasten über `lp_key(…, 1)`, MIDI-In direkt in
  `arp_key`). MODE OFF/UP/DOWN/UP-DN/RAND/ORDER, RATE (`ARP_TICKS`, 1/4 … 1/32), OCT 1–4, GATE 10–100 % (ACID
  darüber `ARP_SLIDE`: Note-on vor Note-off = Legato). Takt: läuft die Schleife, ihr Raster (`lp.pos16`, beim
  Vorzählen `lp.count16`), sonst die gemeinsame Uhr der Arps (`arp_free16`, startet mit dem ersten Arp, die anderen
  reihen sich ein); der erste Schritt wartet `ARP_GATHER` (14,5 ms) auf den Akkord. REC
  nimmt die Arp-Noten auf ihren Ticks auf (`lp_record_at`, ohne QUANT); USB-MIDI-Out auf dem Kanal des Parts.
  Felder in `mod_set_t` (`KLSA`).
- Edit-Seiten (v0.8): ein Sound hat `KL_NV` = 14 Werte (die acht Parameter, dann `KL_ATK`, `KL_DEC`, `KL_SUS`,
  `KL_REL`, `KL_FENV`, `KL_MONO`; `work`/`orig`/`user_t.v`); VA, PD, ORGAN (`kl_env`) haben vier Seiten (`kl_pages`:
  …, ENV, MORE), die anderen zwei. Taster ENV: `env_show` (ACID bleibt Accent, DRUMS `NO ENV`). DSP: `env_tick` mit
  stufenloser DEC/REL (`env_k`, `ENV_FRAC`), Zeiten bis −60 dB `ENV_T60`/`kl_env_ms`; VOICE-Wechsel lässt los, was
  klingt (`ensure_loaded`). Bilder: `draw_env`, `draw_more`, `draw_flt` (VA, 2-poliger SVF), `draw_algo` (FM6,
  `kl_fm6_algo` aus msfas Bus-Flags `fm_algorithm_flags`, `kl_fm6_levels`). Speicherformat `KLSB` (KLSA … KLS7 über
  `user_from8`).
- Effekte (FX; FX/OCT+ wechselt DELAY/HALL; `set.fx[2][4]`, `fx_*` in `app.c`, `dsp/fx.c`): Send-Effekte, die Sends
  mono summiert, die Rückwege stereo vor dem Limiter. DELAY Ping-Pong (erstes Echo links), höchstens 0,5 s je Seite
  (`FX_DLY_N`), TIME zum `lp.tempo`, darüber halbiert (`dly_samples`); HALL = Rings (`set_time`/`set_lp` wie Rings),
  PRE bis 100 ms. Ohne Send schläft ein Effekt, sobald er verklungen ist. Pool: 88 200 + 8 820 + 65 536 B.
- Sounds (v0.9, GLO-Tabs LIST und GLYPHS, `snd_*` und `draw_sounds` in `app.c`): ein Sound ist `SND(e, p)` =
  Engine × 16 + Preset (USER nach den Presets); `snd_list` liefert, was K1 zeigt (`ui.snd_all`: ALL in der
  Reihenfolge des Racks, sonst die Engine des Parts), auf GLYPHS ab `ui.anc` sortiert nach Abstand der Glyphen
  (`kl_shape`, Summe der acht Differenzen, Gleichstand in Listenreihenfolge; `snd_anchor` beim Öffnen des Tabs und
  beim Partwechsel). PRESET dort `snd_step` (auf ALL wechselt die Engine des Parts), K1 `snd_engine` (ALL, dann wie
  `engine_step`), K2–K4 frei, kein HOME-Reset. Gewählter Sound in der Mitte (LIST 7 Zeilen, GLYPHS 3 × 3).
- Looper (SEQ, PLAY, REC; `lp` in `app.c`): Noten, kein Klang; eine Schleife für alle Parts, je Part `lp_ev[p]`
  (2048 Ereignisse, Tick 16 Bit bei 192 je Viertel, höchstens 16 Takte), Rückgängig über `lp_undo` (eine Stufe je Part).
  Läuft im Audio-Interrupt: `app_render` in Schritten zu 32 Frames (`lp_step` vor jedem `kl_render`); die
  Hauptschleife fragt über `lp_ask` (Befehle) und `lp_key` (Tasten), MIDI nimmt `midi_in` direkt auf. Klick im
  Render, Loops nur im RAM.
- Step-Sequencer (Tab STEP der Looper-Seite, `stp` und `step_*` in `app.c`): dieselben Ereignisse wie der Looper,
  ein Step = 1/16 = 48 Ticks (±24); die 16 weißen Tasten (`PLAT_WHITE_KEYS`) sind die Steps des gewählten Parts,
  DRUMS je Instrument (= Halbton); Änderungen über `lp_ask_step` (`LC_SSET`, `LC_SCLR`, `LC_SEDIT`) im Render;
  ALGO = Cursor (`ui.cur`, `step_cursor`, `step_edit_mask`: gehaltene Tasten, sonst der Step des Cursors); HOME + OCT+
  verdoppelt die Schleife mit Inhalt (`LC_DOUBLE`, `lp_double` im Render, Ergebnis `lp.double_res`), HOME + OCT−
  halbiert sie (`loop_half`, wie K2 LENGTH).
- Engines: VA = ANALOG, PD = PHASE, ORGAN = WHEEL, FM6 = msfa (`KL_FM6`), DRUMS (`KL_DRUMS` = 4, Kachel der früheren STRINGS). Je Engine acht Parameter (`engines/src/felucca_engine.h`:
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
- Eigene Dateien MPL-2.0, `dsp/engines.c`, `dsp/drums/`, `dsp/acid/`, `dsp/hall/` und `tools/gen_acid_tables.py` MIT, `dsp/fm6/` und
  `tools/gen_fm6_tables.py` Apache-2.0.
  **Keine GPL-Quellen** übernehmen (z. B. Dexed, Felucca-App-Code, KR-106, Shine On). Übernommen: Felucca-Engines (MIT), msfa (Apache-2.0) für FM6, Plaits-Drums (MIT) für DRUMS,
  Open303 (MIT, aus dem Original-Repo) für ACID, Rings' Hall (MIT) für HALL. Keine Samples aus Rolands Geräten.
- Jede neue Quelle vor der Übernahme in `LICENSING.md` eintragen.
- `id_prefix = "FM-1_77"` ist noch nicht auf Kollisionen geprüft.

## UI-Designsprache
- Raster jedes Screens: Kopfzeile 22 px (Engine-Chip in Engine-Farbe, Glyphe, Soundname, Oktave, Akku) · Inhalt ·
  Fußzeile 32 px (was die Regler gerade halten, je Feld ein Balken).
- Farben (RGB): ground 11,13,17 · head 17,21,28 · surface 21,25,34 · surface2 28,34,48 · line 42,50,66 ·
  text 232,234,237 · muted 154,163,178.
  Engines: FM6 92,200,255 · VA 255,180,84 · PD 255,122,182 · ORGAN 126,224,138 · DRUMS 185,162,255 · ACID 227,244,94.
  Aufnahme (Looper): rec 255,92,92 (`K_REC`).
- **Glyphe:** die acht Parameter eines Sounds als acht Strahlen, im Uhrzeigersinn ab oben, Länge
  r × (0,25 + 0,75 × Wert/255) (Wert = Position im Bereich des Parameters, `kl_shape`), Fläche 3/8 Engine-Farbe über
  dem Hintergrund, Rand 2 px in Engine-Farbe, Mittelpunkt 3×3. Erscheint überall, wo ein Sound genannt wird.
  Maßgeblich ist `glyph_draw` in `app.c`; die Glyphen in den Mockups sind nur Beispiele.
- Kleinste Schrift auf dem Gerät: `FONT_T`; wo möglich `FONT_XS` oder größer.
- Bestätigen / Zurück auf Unterseiten: OCT+ / OCT− (wie in Felucca).
- Regler zurücksetzen (Fabios Vorgabe): **überall, wo ein Regler einen Grundwert hat**, setzt HOME halten + K1–K4 ihn
  darauf zurück, und eine Marke im Balken der Fußzeile zeigt den Grundwert, solange der Regler woanders steht. Edit:
  Wert des Sounds (`orig` in `app.c`); Mixer: 0 dB, Mitte, aus (Marke nur LEVEL und PAN), Sends 0; Looper: 120 BPM,
  Länge der ersten Aufnahme, 1/16, REC; Effekte: `FX_DEF`. Neue Screens bekommen das gleich mit.
- Signatur-Ideen (Klang-Atlas, Glyphen-Browser, Klang-Stammbaum): `docs/CONCEPT.md`, Mockups 08–10.

## Fahrplan
| Version | Inhalt | Stand |
| --- | --- | --- |
| v0 | Rack, VA / PD / ORGAN, Glyphen, Speichern, Host-Tests, CI | erledigt (Simulator) |
| v0.1 | Edit-Seiten (EDIT → 2 Seiten × 4 Parameter auf K1–K4), Live-Glyphe, eigener Sound pro Engine (SAVE), README-Hinweis fribidi/.venv, Tests mit Screenshots | erledigt (Simulator) |
| v0.2 | FM6 (msfa, Apache-2.0, nach C portiert), acht eigene Presets | erledigt (Simulator) |
| v0.3 | DRUMS (Kits 606, 707, 808, 909; Plaits, MIT; ersetzt STRINGS), dann ACID (Open303, MIT) | erledigt (Simulator) |
| v0.5 | Parts (4, je eine Engine; Stimmen-Obergrenze), Mixer (Level, Pan, Mute, Solo, Limiter), Noten-Looper | erledigt (Simulator) |
| v0.6 | Step-Sequencer (auf den Ereignissen des Loopers), Effekte (Delay, Hall), sechs Parts; dazu Step-Cursor, DOUBLE/HALF | erledigt (Simulator; Emulator-Protokolle 29.10.) |
| v0.7 | LFO und Arpeggiator (Taster LFO, ARP; eigener Code) | erledigt (Simulator) |
| v0.8 | Edit-Seiten nach den Mockups 34–36 (aus 02 `Edit-VA`, 03 `Edit-FM6`): Hüllkurve ENV und MORE (Taster ENV), VA-Filterkurve, FM6-Algorithmus | erledigt (Simulator) |
| v0.9 | Preset-Liste und Glyphen-Browser als GLO-Tabs LIST und GLYPHS (Mockups 37, 38) | erledigt (Simulator) |
| v1.0 | Klang-Atlas, Klang-Stammbaum (früher v0.6, dann v0.8) | offen |
| danach | Sequencer-Extras aus Mockup 05 (CHANCE, RATCH, Patterns, SONG), Drive und Chorus | offen |
| am Ende | Feinschliff: Klang nach Gehör, Design-Fragen; Streichen, was nicht trägt | offen |
