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
| ACID | Open303 (Robin Schmidt), schon auf den FM-1 portiert (X0X) | MIT | github.com/mediaworkbench/Felucca (Lizenzhinweis) |
| Juno | KR-106 | GPL-3.0, CPU-hungrig | kayrock.org/kr106, altstrument.org (FM-106) |
| Strings | kein MIT-Kandidat gefunden; Plan: ANALOG + Ensemble-Chorus | – | – |

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
