# Recherche (Stand 26.10.2026)

Zusammenfassung der Recherche aus dem Planungs-Chat (claude.ai, 25.–26.10.2026). Die FM-1-Szene ändert sich fast
täglich: Zahlen und Versionen vor Entscheidungen neu prüfen. Jede Aussage hat ihre Quelle; „unsicher“ heißt: nicht
bestätigt.

## Hardware

| Punkt | Stand | Quelle |
| --- | --- | --- |
| SoC | JieLi AC791N (WL82), zwei pi32v2-Kerne | altstrument.org/devices/m-vave-fm-1 |
| Display | 240×240 Farbe (erste Annahme 320×240 war falsch) | github.com/joakimeriksson/fm1-emu, github.com/seanohollaren/fm1-doom, `base/host/plat_host.c` |
| Bedienung | 27 Tasten, 14 Taster mit LEDs, 7 Encoder (SELECT, ALGO, PRESET, K1–K4), MASTER, TRS-MIDI-In | joakimeriksson.github.io/fm1-emu, `base/include/plat.h` |
| Speicher | 1 MB Flash; für die App: 581 KB Slot, 96 KB RAM (.data/.bss), 336 KB Pool, kein externer RAM | fm1-emu, `base/README.md` |
| Tasten | Anschlagdynamik und Druck auslesbar (`APP_TOUCH`) | `base/README.md` |

## Offizielle Firmware

- **Stock V16 (21.10.2026):** acht Parts multitimbral, 20 Stimmen geteilt, Sequencer bis 64 Steps, RGB-Farbeinstellung
  der Oberfläche. Quellen: pianoandsynth.com (Artikel zu V16), forum.m-vave.com/topic/23.
- Die Community-Werkzeuge und Rückwege sind noch auf **V15** ausgelegt; Faustregel: vor dem Wechsel zwischen
  Custom-Firmwares zurück auf V15. Quellen: altstrument.org, wero1414.github.io/atlas-fm-1.
- **FM1 Open:** offizielles Open-Source-Projekt von M-VAVE; App-Code GPL-3.0, die Sound-Engine nur als
  Binär-Bibliothek, Build unter Windows. Quellen: github.com/m-vave/fm1_open, synthtopia.com (20.10.2026).

## Community-Firmwares mit klassischen Klängen (Auswahl)

| Firmware | Inhalt | Lizenz | Quelle |
| --- | --- | --- | --- |
| Felucca | Basis vieler Forks, 13 Engines, Web-Editor, FM6 auf Dexed/msfa | GPL-3.0 (5–6 Engines MIT) | github.com/hugelton/Felucca |
| Melodee | Prophet-5, CZ-1, FM6 mit DX7-Bänken | GPL-3.0 | github.com/keremimo/melodee |
| FM-106 | Juno-106 (Port von KR-106), 6 Stimmen auf beiden Kernen | – | altstrument.org |
| Clipper | 23 Engines, 407 Sounds, UNDO/COMPARE/MORPH | GPL-3.0 | github.com/isod89/clipper-fm1 |
| Skiff | 6-Track-Groovebox, 8 Undo-Stufen, Sound-Typen | GPL-3.0 | github.com/illuminus5/skiff |
| DUELL | bitimbral, MORPH zwischen zwei Versionen eines Sounds | GPL-3.0 | github.com/oscillr/DUELL |

Verzeichnisse: altstrument.org (rund 227 Firmwares, Stand 26.10.), github.com/aroum/fm1-custom-fw,
fwradar.com/p/m-vave-fm-1/custom-firmware, fm1-editor.com/firmware, wero1414.github.io/atlas-fm-1.

## Basis-Vergleich (Entscheidung: fm1-base, siehe DECISIONS.md)

- **Felucca-Fork:** schnellster Weg zu vielen Sounds, fertiges Bedienkonzept; dafür GPL-3.0 und ein dichtes
  bestehendes UI-Konzept.
- **fm1-base:** Plattformschicht, Host-Simulator, Browser-Emulator, Build-Werkzeuge; MPL-2.0, die App darf jede
  Lizenz haben; Beta (Schnittstellen können sich vor 1.0 ändern). Quelle: github.com/charlesvestal/fm1-base.
- **fm1-dock:** Synths als Module im Flash, Auswahl am Gerät; GPL-3.0 (auf Dinghy); jedes Modul mit eigener UI.
  Quelle: github.com/joakimeriksson/fm1-dock.

## Engine-Quellen und Lizenzen

| Klang | Quelle | Lizenz | Quelle der Angabe |
| --- | --- | --- | --- |
| VA, PD, ORGAN (+ LOFI, TRIO, NOISE) | Felucca-Engines | MIT (PHASE im Code geprüft) | github.com/hugelton/Felucca-Engines |
| FM6 | msfa (Kern von Dexed) | Apache-2.0; Dexed selbst GPL-3.0 | github.com/asb2m10/dexed |
| ACID | Open303 (Robin Schmidt), schon auf den FM-1 portiert (X0X, in einem GPL-Fork: nicht von dort übernehmen) | MIT | github.com/RobinSchmidt/Open303 (`License.txt`), github.com/mediaworkbench/Felucca (Lizenzhinweis) |
| DRUMS | Plaits-Drums (Mutable Instruments) und eigener Code | MIT | github.com/pichenettes/eurorack, siehe „DRUMS“ unten |
| Juno | KR-106 | GPL-3.0, CPU-hungrig | kayrock.org/kr106, altstrument.org (FM-106) |
| Strings | kein MIT-Kandidat gefunden; Plan war ANALOG + Ensemble-Chorus (am 26.10. durch DRUMS ersetzt) | – | – |

## FM6: msfa (Stand 26.10.2026)

- **Quelle:** github.com/google/music-synthesizer-for-android, Ordner `app/src/main/jni/`. Lizenz Apache-2.0 (laut
  GitHub-API; Lizenztext `COPYING`, byteweise gleich mit apache.org/licenses/LICENSE-2.0.txt), Repo archiviert,
  letzter Push 23.11.2021, keine `NOTICE`-Datei im Stamm. Dateiköpfe: „Copyright 2012/2013 Google Inc“.
- **Übernommen** (nach C portiert, `firmware/klassik/dsp/fm6/msfa.c`): dx7note, env, pitchenv, lfo, fm_core,
  fm_op_kernel (portabler Teil), sin, exp2, freqlut. Nicht übernommen: synth_unit (MIDI, Stimmen), Filter, NEON.
- **Warum C:** `base/tools/build.py` übersetzt nur C (`-std=c99` bzw. `gnu99`) und meldet „soft-double routines“
  als Fehler; msfa ist C++ und füllt seine Tabellen beim Start mit `double`.
- **Gerät:** `base/bsp/docs/CPU.md`: Inline-64-Bit-Multiplikationen neben 16-/32-Bit-Ladebefehlen lassen auf dem
  FM-1 (360 MHz) gelegentlich ein Register um ein Bit falsch; Abhilfe: Multiplikation in einer `noinline`-Funktion.
  msfa multipliziert pro Sample mit 64 Bit (`sin.h`, `fm_op_kernel.cc`). Die Felucca-Engines nutzen kein `int64_t`.
- **Prüfung des Ports** (Mac, 64 zufällige gültige DX7-Stimmen, alle Algorithmen und Feedback-Stufen, je 64 000
  Samples, Block 32): Tabellen bitgleich (20 484 B); mit exakter 64-Bit-Gain-Multiplikation ist der Port
  **bitgleich** mit dem Original (16 384 000 B). Mit der 32-Bit-Gain-Multiplikation des Ports: Rauschabstand je
  Stimme im Median 89,6 dB, schlechtester Fall 62,2 dB (Feedback 7). Nur Simulator, nicht auf Hardware.
- **Achtung beim Vergleichen:** Auf arm64 (Apple Silicon) nimmt msfa seine NEON-Kernel (`synth.h`: `__aarch64__`),
  die vom portablen Pfad abweichen (Median 54 dB Rauschabstand untereinander). Referenz ist der portable Pfad.
- **Tonhöhe** (Simulator, `tests/fm6.txt`-Aufnahmen): C4 auf E.PIANO 261,88 Hz (Soll 261,63 Hz, DTUN 2),
  FM ORGAN 130,83 Hz (16', Soll 130,81 Hz).
- **Knacken beim Wiederanschlag** (Fabios Browser-Test 26.10., `docs/tests/FM6.md`: „knackt“ bei sehr schnellem
  Spiel): msfas Note-Start (Port: `fm_note_init`) setzt bei jedem Operator Phase und Hüllkurve auf 0, und der
  Stimmen-Host gibt einem noch klingenden Ton dieselbe Stimme (`dsp/engines.c`, „the same note again“). Gemessen im
  Simulator (Töne über MIDI, ca. 40 ms Abstand; gezählt: Samples, deren zweite Differenz mehr als das Fünffache der
  größten in den 10 ms davor ist):

  | Fall | vorher | mit `fm_note_retrigger` |
  | --- | --- | --- |
  | C4 16-mal, GLASS BELL / FM ORGAN | 15 / 15 | 0 / 0 |
  | Akkord C-E-G 12-mal: E.PIANO, GLASS BELL, BRASS, MARIMBA | 11, 9, 11, 9 | 4, 0, 0, 2 |
  | Akkord 12-mal: FM BASS, FM ORGAN, WARM PAD, CLAV | 0 | 0 |
  | Glissando aus 16 Tönen (ab dem 7. gestohlene Stimmen), GLASS BELL / FM ORGAN | 0 / 0 | 0 / 0 |

  Die übrigen Treffer sind kleine Anschläge nach fast völliger Stille (zweite Differenz 21–42, davor 2–4; vorher an
  denselben Stellen bis 204). Erster Versuch (jeder Operator macht weiter) machte FM BASS schlechter (0 → 5), weil
  ein schon unhörbarer Ton dann an alter Phase statt bei 0 neu einsetzt; daher nur Operatoren über −42 dB
  (`RETRIG_KEEP`). Frische Noten laufen wie bisher über `fm_note_init` (unverändert). Nur Simulator.
- **Browser-Emulator:** rechnet das ganze Gerät im Audio-Thread, je 128 Samples, auch das Neuzeichnen des
  Bildschirms (`base/web/emu/emu_web.c` `web_render`, `worklet.js`). Ob das bei schnellem Spiel Aussetzer macht, ist
  nicht gemessen; die Seite zeigt die mittlere Last („… uses …% of this computer's time“).

## DRUMS und ACID: Quellen (Stand 26.10.2026)

- **Die Vorbilder:**
  - TR-909: Bassdrum, Snare und Toms analog, Hi-Hats, Ride und Crash als 6-Bit-Samples
    (attackmagazine.com, Interview mit Atsushi Hoshiai).
  - TR-707: alle Klänge Samples, 8 Bit, Crash und Ride 6 Bit, 25 kHz (vintagesynth.com/roland/tr-707).
  - TR-808: Hi-Hats und Becken aus sechs Rechteck-Oszillatoren (Hex-Inverter), etwa 200–800 Hz
    (docs.cherryaudio.com/misfit-audio/transistor-808-hats); die genauen Frequenzen streuen von Gerät zu Gerät.
  - TR-606: analog laut allgemeiner Kenntnis; eine belastbare Quelle zum Aufbau fehlt noch (unsicher).
  - Samples aus den Geräten übernehmen wir nicht (Rechte bei Roland); 707 und die Becken der 909 werden synthetisch
    angenähert.
- **Plaits** (github.com/pichenettes/eurorack, `plaits/dsp/drums/`):
  - `analog_bass_drum.h` („808 bass drum model, revisited“), `analog_snare_drum.h` (808), `hi_hat.h` (808, mit
    Parametern „to push things to the CY territory“), `synthetic_bass_drum.h` („Inadvertently 909-ish“),
    `synthetic_snare_drum.h` (Werte aus den 909-Schaltplänen, z. B. Verhältnis der Moden 1,47).
  - Lizenz: MIT im Kopf jeder Datei („Copyright 2016 Emilie Gillet“) und in der README („Code (STM32F projects):
    MIT license“; AVR-Projekte GPL-3.0, Plaits ist STM32). Die GitHub-API meldet keine Lizenz (keine `LICENSE`-Datei
    im Stamm).
  - C++ mit `float` und stmlib (Filter, Rauschen): nach C zu portieren. Der AC79 hat eine FPU für einfache
    Genauigkeit (`base/tools/build.py:113`), `double` bleibt verboten.
  - Hi-Hat-Verhältnisse der sechs Oszillatoren in `hi_hat.h`: 1,0 · 1,304 · 1,466 · 1,787 · 1,932 · 2,536.
- **Open303** (github.com/RobinSchmidt/Open303): MIT laut `License.txt` („Copyright (c) 2009 Robin Schmidt“); die
  GitHub-API meldet „NOASSERTION“, letzter Push 29.03.2024. Rechnet in `double`: nach C mit `float` zu portieren.
- **Marken:** Roland hat 2019 in Deutschland die Designs von TR-808 und TB-303 als Marken angemeldet (factmag.com/roland,
  synthanatomy.com, 02/2019). Ob die Nummern als Wortmarken geschützt sind, ist nicht geklärt (DPMA, USPTO prüfen).

## DRUMS: Port und Messungen (Stand 26.10.2026, nur Simulator)

- **Port gegen Plaits:** Plaits' Original (C++, eurorack `f4cfa76`, stmlib `d18def8`) und der C-Port
  (`firmware/klassik/dsp/drums/drums.c`) auf dem Mac nebeneinander, beide 48 kHz, Blöcke zu 32, je 2 s mit einem
  zweiten Schlag nach 1 s, zwei Parametersätze je Modell. Rauschabstand Port gegen Original:

  | Modell | Satz 1 | Satz 2 |
  | --- | --- | --- |
  | 808-Bassdrum | 94,3 dB | 98,8 dB |
  | 909-artige Bassdrum | 68,6 dB | 64,1 dB |
  | 808-Snare | 78,0 dB | 75,2 dB |
  | 909-artige Snare | 78,0 dB | 72,3 dB |
  | 808-Hi-Hat | 78,3 dB | 55,7 dB |

  Nicht bitgleich, weil der Port eigene Näherungen nutzt (`exp2` bis 1,5·10⁻⁴, `sin` bis 0,1 %, `sqrt`) und den
  Zufallswert auf 24 Bit kürzt statt rundet. Der erste Vergleich zeigte bei der 909-Bassdrum nur 10 dB: stmlibs
  `ParameterInterpolator` übernimmt am Blockende den erreichten Wert, nicht das Ziel, und zählt während des
  FM-Pulses nicht weiter; nachgebaut.
- **Pegel** (je Instrument ein Schlag, Velocity 100, MASTER 50 %, `DM_NORM` in `eng_drums.c`; Stand 27.10.):
  Bassdrums −22,7 bis −26,1 dBFS, Snares −25,1 bis −27,2, Toms −25,2 bis −26,6, Clap −25,9 bis −28,3, Rimshot und
  Cowbell −28,6 bis −29,7, Hi-Hats −29,2 bis −31,2, Becken −30,7 bis −31,9, G# (Ride, Tamburin, Claves) −30,1 bis
  −31,3. Zum Vergleich: eine FM6-Stimme −29,3, eine VA-Stimme −31,5.
- **Accent:** 808-Bassdrum bei Velocity 127 −21,5 dBFS, bei 30 −26,7 dBFS (`tests/drums.txt`).
- **Ausklang** (bis 40 dB unter der Spitze): CH 65–84 ms, OH 430–623 ms, Snares 160–274 ms, Becken 1,07–1,11 s,
  808-Bassdrum 1,17 s (BD DEC 70), 909-Bassdrum 242 ms (BD DEC 50); Toms (eigenes Modell) 435–578 ms.
- **Nach Fabios Test (27.10., `docs/tests/DRUMS.md`):**
  - Toms „klingen nach Bassdrum“: Sie nutzten das 808-Bassdrum-Modell. Jetzt ein eigenes Modell: Sinus, leicht
    gesättigt, der nach dem Anschlag 20–35 % höher einsetzt und in ~40 ms auf seinen Ton fällt, dazu ein kurzes
    Stockgeräusch. Gemessen z. B. 808-MT 162 → 135 Hz, 909-LT 135 → 100 Hz (Nulldurchgänge, 3–25 ms und
    150–250 ms nach dem Schlag).
  - OH und Becken „blechern“: Das Farbfilter der Hats liegt bei 150 Hz · 2^(6 · tone); HAT TONE 50 ergab 1,2 kHz,
    das Becken 0,6 kHz (schmales Band, hoch gütig: ein Blechklang). HAT TONE deckt jetzt 1,9–9,6 kHz ab, die Becken
    sitzen bei 3,9–5,2 kHz. Spektraler Schwerpunkt der ersten 46 ms bei den Kit-Werten: CH 9,7–11,2 kHz,
    OH 8,8–11,2 kHz, CY 9,4–11,5 kHz (vorher OH bei HAT TONE 50: 5,3 kHz).
  - SD TUNE und TOM TUNE „ändern nichts“: Im Simulator verschieben beide um genau eine Oktave (Snare 90 / 180 /
    360 Hz, Tief-Tom 48 / 94 / 190 Hz bei −12 / 0 / +12; `drums-tune.wav`). Vermutung, nicht geprüft: Laptop-
    Lautsprecher geben die Grundtöne unter ~200 Hz kaum wieder.
- **Tonhöhe** (Autokorrelation, 60–260 ms nach dem Schlag): Bassdrums und Toms von 606, 707 und 808 innerhalb von
  1–2 % des Kit-Werts (z. B. 808-BD 52,2 Hz, Soll 52); die 909-artigen Toms 2–3 % höher (Rest des Pitch-Sweeps).
- **Speicher:** Zustand je Stimme höchstens 140 B (808-Snare: sechs Filter) plus ~20 B, sechs Stimmen also rund
  1 KB RAM. Code: `drums.c` 8,1 KB auf dem Mac (`size`, arm64, -O2); auf dem AC79 unbekannt.
- **Rechenlast (Schätzung, unsicher):** Die 808-Snare ist das teuerste Modell, mit etwa 115 Gleitkomma-Operationen
  und einer Division je Sample (fünf Resonatoren und das Rauschfilter zu je ~10 Operationen). Bei 1 bis 2,5 Takten
  je Operation und ~15 Takten je Division kostet eine Stimme 130 bis 300 Takte je Sample. Mal 44 100 Samples je
  Sekunde sind das 5,7 bis 13,2 Mio. Takte je Sekunde. Sechs Snare-Stimmen auf einmal (der ungünstigste Fall)
  brauchen 34 bis 79 Mio. Takte, das sind 10 bis 22 % von 360 MHz. Messbar erst mit Gerät.

## Neuheits-Check der Signatur-Ideen

Klang-Atlas, Glyphen und Klang-Stammbaum wurden in den genannten Verzeichnissen nicht gefunden (Stand 26.10.).
Verwandtes gibt es: UNDO/COMPARE/MORPH (Clipper), MORPH (DUELL), Undo-Stufen (Skiff), Kategorien/Favoriten (Melodee,
Skiff). Jede einzelne Firmware wurde nicht geprüft: Neuheit **unsicher**.

## Entwicklungsumgebung (Mac M3)

- JieLi-Toolchain ist nur Linux x86-64: Geräte-Build in einem linux/amd64-Docker-Container (Rosetta).
  Quellen: github.com/charlesvestal/fm1-loom/blob/main/BUILDING.md, github.com/isod89/sloop-fm1/blob/main/BUILDING.md.
- Host-Simulator baut nativ mit `cc` (bei uns getestet), Browser-Emulator braucht Emscripten.
- Fonts: Pillow braucht libraqm; die Pillow-Wheels laden libfribidi zur Laufzeit, auf Apple Silicon unter
  `/opt/homebrew/lib` nur mit `DYLD_FALLBACK_LIBRARY_PATH`. Quellen: pillow.readthedocs.io (Building from source),
  Pillow-Quellcode 11.3.0 `src/thirdparty/fribidi-shim/fribidi.c`, fm1-loom BUILDING.md.
- PyCharm hat keine C/C++-Unterstützung, CLion schon (frei für nicht-kommerzielle Nutzung).
  Quelle: intellij-support.jetbrains.com (Thread „PyCharm from CLion“).
- Claude Code: Plugin für JetBrains-IDEs, lädt `CLAUDE.md` zu Beginn jeder Sitzung.
  Quellen: code.claude.com/docs/en/jetbrains, code.claude.com/docs/en/memory.
- Zum Prüfen auf echter Hardware später: **Mocky** zeigt Bilder per USB auf dem 240×240-Display; **fm1-emu** führt
  `.fwsc`-Pakete im Browser und nativ auf Apple Silicon aus.

## Sicherheit beim Flashen (für später)

- Single-Bank-Flash, kein etablierter Rückweg bei abgebrochenem Schreiben (altstrument.org).
- Jede Firmware muss in den Update-Modus zurückfinden, sonst ist das Gerät für Updates gesperrt
  (github.com/aroum/fm1-custom-fw). fm1-base bringt diesen Weg mit; nie entfernen.
