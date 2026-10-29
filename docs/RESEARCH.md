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
- **Browser-Emulator, Pfeiltasten:** Ein Klick fokussiert den Regler (`hit`, `pointerdown` → `focus`); ein
  fokussierter Regler nimmt ↑ / ↓ / ← / → selbst und reicht sie nicht weiter (`stopPropagation`), nur sonst drehen ↑ / ↓
  SELECT (`base/web/emu/index.html`, Zeilen 236–237, 375–381, 449). Nach einem Klick auf K1 im Rack wechseln ↑ / ↓
  also die Engine. Vermutliche Ursache von LOOPER C4 (29.10.2026, nicht nachgestellt).

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

## ACID: Open303, Port und Messungen (Stand 27.10.2026, nur auf dem Mac)

- **Lizenzprüfung** (github.com/RobinSchmidt/Open303, Stand `313bf0d`, 29.03.2024):
  - `License.txt`: „The Open303 source code is released under the terms of the MIT license“, Copyright (c) 2009 Robin
    Schmidt. Die Dateien in `Source/DSPCode/` haben keine eigenen Lizenzköpfe.
  - Die Suche nach „GPL“, „GNU“, „license“ und „copyright“ in `Source/` findet nur das Compiler-Makro `__GNUC__`.
    Die Dateien sind ISO-8859 kodiert, deshalb mit `grep -a` gesucht: Ein einfaches `grep` überspringt sie als
    Binärdateien.
  - Nicht übernommen: `Libraries/` (VST-SDK 2.4 von Steinberg, CLAP), `fft4g.c` (Ooura; die Tabellen rechnet ein
    eigenes Python-Werkzeug), `rosic_AcidSequencer`, `rosic_AcidPattern` (Sequencer, vielleicht für v0.5), das
    VST-Plugin.
- **Fakten aus dem Code**, wichtig für die Makros:
  - Accent: Eine Note mit Velocity ≥ 100 ist akzentuiert (`rosic_Open303.cpp`, `noteOn`).
  - Slide: Eine Note, die kommt, solange eine andere gehalten ist, gleitet (`noteOn`: Notenliste nicht leer →
    `slideToNote`). Wird die letzte Taste losgelassen und ist noch eine gehalten, gleitet der Ton zu ihr zurück
    (`releaseNote`).
  - `setSlideTime(ms)` setzt die Zeitkonstante des Slew-Limiters auf 0,2 × ms („\todo: tweak the scaling
    constant“). Der Konstruktor setzt sie direkt auf 60 ms. Das Plugin hat keinen Slide-Regler, also gelten dort
    60 ms.
  - Die Bereiche im Plugin (`Open303VST.cpp`): CUTOFF 314–2394 Hz exponentiell, DECAY 200–2000 ms exponentiell,
    RESONANCE, ENVMOD und ACCENT 0–100 %, WAVEFORM 0–1 (Säge → Rechteck), TUNING 400–480 Hz.
  - Filter: Der Kern nimmt den Modus `TB_303` (`rosic_TeeBeeFilter.cpp`, Konstruktor: „ala mystran & kunn“). Das
    Plugin stellt standardmäßig `LP_18` ein (`Open303VST.cpp:23`). Beide Modi sind portiert.
  - Fest im Kern: Amplituden-Hüllkurve mit Decay 1230 ms, Sustain 0 und Release 1 ms (mit Accent 50 ms);
    Accent-Decay 200 ms; Hochpass 44,486 Hz vor und 24,167 Hz nach dem Filter; 150 Hz in der Rückkopplung; Notch
    7,5164 Hz; 4-fache Überabtastung mit einem elliptischen Filter 12. Ordnung.
- **Speicher** der Wellenform-Tabellen:
  - Original: 2 Wellenformen × 12 Tabellen × 2052 Werte × 8 B (double) = 393 984 B.
  - Port: Es braucht nur die Tabellen 0–9, denn eine Note unter 20 kHz erreicht höchstens Tabelle 9. Die Rechnung:
    Inkrement 2048 × 20 000 / 176 400 = 232, Exponent 7, plus 2 ergibt Tabelle 9.
  - Ab Tabelle 5 ist jede halb so lang wie die vorige. Sie hat auch nur halb so viele Obertöne, also bleiben
    mindestens 32 Werte je Periode des höchsten.
  - Gespeichert als int16 mit einem Wert Überhang: 5 × 2049 + 1025 + 513 + 257 + 129 + 65 = 12 234 Werte je
    Wellenform. × 2 Wellenformen × 2 B = **48 936 B im Flash**.
  - RAM: ein `acid_t` mit gut 50 floats, etwa 200 B.
- **Tabellen gegen das Original:** `gen_acid_tables.py` (Python, double) gegen die `tableSet`-Werte von Open303:
  höchstens 2,6e-14 Abweichung je Wert in allen 20 Tabellen. Das elliptische Filter als sechs Biquads trifft den
  Frequenzgang der Direktform auf 1e-6 (das prüft der Generator selbst).
- **Port gegen das Original:** Open303 (C++, double, `Source/DSPCode/` unverändert bis auf die x86-Assembler-Zweige)
  und der C-Port (`firmware/klassik/dsp/acid/acid.c`, float) auf dem Mac nebeneinander, 44,1 kHz, dieselben Noten
  über die Notenliste von `noteOn`, je 1–2 s. Rauschabstand Port gegen Original:

  | Fall | Einstellung | Rauschabstand |
  | --- | --- | --- |
  | vst_default | Plugin-Standard (Cutoff 500 Hz, Reso 50 %, EnvMod 25 %, Decay 400 ms), TB_303 | 60,9 dB |
  | saw_dark | Cutoff 314 Hz, Reso 0, Säge | 66,0 dB |
  | square_open | Cutoff 2394 Hz, Reso 90 %, EnvMod 100 %, Rechteck, Accent | 64,2 dB |
  | slide_up | 36 → 48 gebunden, Accent | 64,6 dB |
  | slide_back | 36 → 43 → zurück zu 36, Slide 300 ms | 63,2 dB |
  | retrigger | Wiederanschlag ohne Zurücksetzen, Accent | 62,4 dB |
  | high_note | Note 96 (Tabelle 7) | 67,2 dB |
  | low_selfosc | Note 24, Reso 100 % (Filter schwingt selbst) | 56,3 dB |
  | pattern | zwei Sekunden Linie mit Slides und Accents | 61,9 dB |
  | lp18_default | Plugin-Standard mit LP_18 | 63,3 dB |
  | lp18_squelch | LP_18, Reso 90 %, Accent | 59,6 dB |

- **Was in float nicht hielt** (gemessen, dann behoben):
  - Der Slew-Limiter blieb 0,004 Hz vor dem Ziel stehen, bei 130 Hz sind das 3e-5. Unterhalb davon geht ein Schritt
    in der Rundung unter. Jetzt wird der Abstand zum Ziel geführt; vorher 49,3 dB in slide_up.
  - Koeffizienten knapp unter 1, zum Beispiel 0,99962 für den Slide: In float ist die Zeitkonstante dann um bis zu
    1,6e-4 ungenau. Jetzt wird 1 − Koeffizient gespeichert; vorher 52,6 dB in slide_back.
  - Das elliptische Filter in Direktform II hat Koeffizienten bis 308. In float ist das nicht stabil zu halten,
    deshalb Biquads (nicht gemessen, sondern vorab ersetzt).
  - Die verkürzten Tabellen kosten nur bei hohen Noten etwas: high_note 67,2 statt 72,2 dB mit allen Tabellen in
    voller Länge.
- **Rechenlast (Schätzung, unsicher):** etwa 500 Gleitkomma-Operationen und eine Division je Sample.
  - Davon 4 × ~95 überabgetastet: das elliptische Filter mit 54 je Schritt ist der größte Teil, dazu Oszillator,
    Hochpass und Filter.
  - Bei 44,1 kHz sind das ~22 MFLOP/s. Mit 2–3 Takten je Operation ergibt das 44–66 Mio. Takte, also 12–18 % von
    360 MHz, für eine Stimme. Auf dem Gerät messen.

- **Pegel im Simulator** (`AC_LEVEL` = 2 × VOICE_FS, MASTER 50 %, Note F3, 400 ms gehalten; Messung mit
  `peak_db_max`), normal / mit Accent in dBFS:

  | Preset | normal | Accent |
  | --- | --- | --- |
  | ACID LINE | −27,3 | −17,8 |
  | SQUELCH | −27,2 | −16,2 |
  | DEEP SAW | −28,6 | −22,0 |
  | SQUARE | −33,0 | −24,8 |
  | RUBBER | −32,1 | −24,6 |
  | SCREAM | −28,2 | −17,2 |
  | PLUCK | −27,2 | −18,8 |
  | SLOW SLIDE | −29,9 | −21,3 |

  Der lauteste Fall (SCREAM mit Accent, fünf Tasten je Oktave, MASTER 50 %), je Oktave −2 … +2: −15,1, −16,2,
  −15,2, −9,1, −6,6 dBFS. Bei vollem MASTER sind das 12 dB mehr (0,25 → 1). Damit übersteuern die Oktaven +1 und +2
  um bis zu 5,6 dB, der Bassbereich nicht. Höher klingt lauter, weil die Resonanzspitze dort die Obertöne trifft.

## Geräte-Build (Stand 27.10.2026, ohne Paket)

- **Toolchain:** JieLis Linux-Toolchain (clang 4.0.1 für pi32v2, Version `jieli-linux-toolchains-20260730.1`), mit
  `base/tools/get_toolchain.sh` nach `~/.jieli/toolchain` geladen (24,8 MB, entpackt 135 MB). Sie läuft im
  Docker-Container `debian:bookworm-slim` (linux/amd64) auf dem Mac (Docker 29.6.2, arm64).
- **SDK:** Das Paket (`.fwsc`) braucht drei Dateien aus JieLis AC79-SDK (`uboot.boot`, `cfg_tool.bin`,
  `eq_cfg_hw.bin`). Laut `base/bsp/tools/get_sdk.sh` kommt das SDK aus github.com/Jieli-Tech/fw-AC79_AIoT_SDK; das
  Repo gibt am 27.10.2026 „Repository not found“ (GitHub-API: 404).
  - Kopien mit genau dem getesteten Commit `d179b448` („AC79NN_SDK_V1.2.1_2023-12-13“): amitv87, msr002, HQWYFHJQ,
    sqtech-ai (je `/fw-AC79_AIoT_SDK`). Nicht geladen: Das ist Fabios Entscheidung (offener Punkt in DECISIONS.md).
    `get_sdk.sh` prüft die benutzten Dateien per SHA-256.
- **Ohne SDK prüfbar:** `firmware/klassik/tools/device_check.py` führt die Schritte von `base/tools/build.py` bis
  zu dessen Prüfungen aus und schreibt kein Paket. Ergebnis für KLASSIK v0.3 mit Regler-Reset:

  | Was | belegt | Budget | frei |
  | --- | --- | --- | --- |
  | App (Flash) | 204 164 B | 581 564 B | 377 400 B (65 %) |
  | RAM `.data` + `.bss` | 35 052 B | 98 304 B | 63 252 B (64 %) |
  | Pool | 69 384 B | 344 064 B | 274 680 B, davon 8 192 B Reserve (`check`): 266 488 B nutzbar |

  Bestanden: kein `double` (keine Soft-Double-Routinen), keine 64-Bit-Multiplikation neben einem Ladebefehl,
  RAM-Code ohne Aufrufe, Registerzugriffe nur in felucca-bsp.
- **Gefunden und behoben:** `dsp/engines.c` hatte `#include <string.h>` und `memmove`. Der FM-1 hat keine libc: Es
  gibt nur `memset`, `memcpy` und `memcmp` (`base/bsp/chip/libc.c`, freestanding). Das fiel bisher nicht auf, weil
  nur Host und Web gebaut wurden.
- **Audio-Looper, nachgerechnet:** 266 488 B Pool / (44 100 × 2 B/s) = 3,0 s mono mit 16 Bit.

## Parts: Kosten je Engine und Speicher (Stand 27.10.2026)

- **Kosten je Stimme**, gemessen im Host-Simulator auf dem Mac (M3, `-O2`). `kl_render` über 4000 Blöcke (2,9 s Klang),
  das Beste aus drei Läufen. ANALOG mit SOFT PAD, weil SAW LEAD einstimmig ist; DRUMS mit sechs verschiedenen
  Instrumenten, weil jedes nur eine Stimme hat. Die Werte sind Mac-Zeit, nur das Verhältnis zählt:

  | Engine | 1 Stimme | 6 Stimmen | je Stimme | relativ zu FM6 |
  | --- | --- | --- | --- | --- |
  | ANALOG | 1,66 ms | 5,27 ms | 0,88 ms | 0,8 |
  | PHASE | 0,58 ms | 3,07 ms | 0,51 ms | 0,45 |
  | WHEEL | 1,47 ms | 7,14 ms | 1,19 ms | 1,05 |
  | FM6 | 1,27 ms | 6,79 ms | 1,13 ms | 1,0 |
  | DRUMS | 3,02 ms (BD) | 7,88 ms (5 klangen) | ~1,6 ms | ~1,4 |
  | ACID | 8,38 ms | (einstimmig) | 8,38 ms | 7,4 |

  Daraus die Kosten in `engines.c` (`COST`, Zehntel einer FM6-Stimme). Die Obergrenze für alle Parts zusammen ist
  `KL_BUDGET` = 240, also 24 FM6-Stimmen.
  - Nach der bisherigen Schätzung (FM6 mit sechs Stimmen 7–13 % von 360 MHz) entspricht das 28–52 % der CPU. Das ist
    **unsicher**, solange es nicht auf dem Gerät gemessen ist.
- **RAM mit vier Parts** (`tools/device_check.py`): `.data` + `.bss` von 35 052 B auf 53 040 B von 98 304 B. Das
  sind 18 KB für die drei zusätzlichen Parts (Stimmen, Zustände der Engines, je ein Ausgabepuffer). App 207 704 B.

- **Sechs Parts** (28.10.2026, `tools/device_check.py`): RAM `.data` + `.bss` 61 784 → 73 864 B von 98 304 B
  (+12 080 B für zwei Parts, geschätzt aus den vier Parts: 2 × 6 000 B); Pool 272 900 → 289 284 B von 344 064 B
  (+2 × 8 192 B Looper-Spuren, 2 048 Ereignisse × 4 B); App 234 128 B. Die Obergrenze (`KL_BUDGET` 240) gilt für alle
  Parts zusammen: mehr Parts heißt nicht mehr Stimmen.
- **Rückgängig je Part** (29.10.2026, `tools/device_check.py`): `lp_undo` je Part statt einmal, +5 × 2 048 × 4 B =
  40 960 B; Pool 289 284 → 330 244 B von 344 064 B (gemessen wie gerechnet), frei 13 820 B; RAM 73 864 → 73 880 B
  (Zähler und Länge je Part im Zustand des Loopers); App 234 128 → 234 220 B.

## Mixer: Messungen (Stand 27.10.2026, nur Simulator)

- **Pegel:** LEVEL ist eine Tabelle 2048 × 10^(dB/20) (Q11) für −40 … +6 dB. Part 1 (VA, SAW LEAD, MASTER 50 %) misst
  bei 0 dB −31,5 dBFS wie vorher, bei −6 dB −37,6 dBFS (1026 / 2048 = −6,02 dB).
- **Pan:** Ganz links ist der rechte Kanal 0, links ~110 000 bei −6 dB; ein Viertel rechts (R25) gibt rechts voll,
  links halb.
- **Gleiten:** Jede Verstärkung gleitet mit einer Zeitkonstante von 256 Samples (5,8 ms). Ganz bei 0 ist sie nach
  ln(2048) × 256 ≈ 1950 Samples (~44 ms).
- **Limiter:** ACID SCREAM, +6 dB, voller MASTER, akzentuierte Noten 84, 90 und 96. Der Ausgang bleibt bei −0,5 dBFS,
  ohne Limiter wäre er bis ~6 dB darüber. Die Decke ist 8 388 607 × 10^(−0,5/20) = 7 919 000. Die Verstärkung
  rechnet ohne 64-Bit-Division: (7 919 000 · 16) / (Spitze / 4096) in 32 Bit.
- **Speicher** (`tools/device_check.py`): App 210 964 B, RAM 53 200 B (+160 B gegenüber den Parts).

## Looper: Rechnungen und Messungen (Stand 27.10.2026, nur Simulator)

- **Zeitraster:** 192 Ticks je Viertel, 4/4, höchstens 16 Takte = 16 × 4 × 192 = 12 288 Ticks (passt in 16 Bit). Ein
  Tick dauert bei 120 BPM 60 000 / (120 × 192) = 2,6 ms, bei 240 BPM 1,3 ms.
- **Genauigkeit:** `app_render` rendert den Block (256 Frames) in acht Schritten zu 32 Frames; vor jedem Schritt
  spielt der Looper die fälligen Noten. Eine Note kommt also bis zu 32 / 44 100 = 0,73 ms zu früh. Aufgenommen wird
  die Position am Anfang des Blocks, in dem die Note auch klingt (Tasten über die Warteschlange, MIDI direkt).
  `kl_render` mit 32 statt 256 Frames achtmal aufzurufen kostet je Aufruf vier `ensure_loaded` und die Leerung der
  Tasten-Warteschlange; wie viel das auf dem Gerät ist, ist **ungemessen**.
- **Fortschritt je Schritt** (Ticks, Q16): BPM / 60 × 192 × 32 / 44 100 × 65 536 = BPM × 152,174; im Code
  BPM × 152 174 / 1000 in 32 Bit (240 × 152 174 = 36,5 Mio. < 2³¹). Abweichung vom exakten Wert 152,1743: 2 · 10⁻⁶.
- **Speicher:** je Part 2048 Ereignisse zu 4 B (Tick 16 Bit, Note, Velocity), dazu ein Puffer zum Zurücknehmen:
  (4 + 1) × 2048 × 4 B = 40 960 B im Pool. `tools/device_check.py`: Pool 69 384 → 110 344 B (+40 960, wie
  gerechnet) von 344 064 B; RAM 53 200 → 55 200 B; App 210 964 → 219 364 B. Eine Spur fasst 1024 Noten
  (an und aus); 16 Takte mit 1/16-Hi-Hats, Kick und Snare auf jedem Schlag sind 16 × (16 + 4 + 4) = 384 Noten.
- **Klick:** Sinus (Magic-Circle-Oszillator in 32-Bit-Festkomma), 1,5 kHz auf der Eins, 1 kHz sonst, fällt mit
  τ = 512 Samples (11,6 ms), nach ~40 ms −30 dB. Gemessen im Simulator bei MASTER 50 %: Eins 158 141 → −34,5 dBFS,
  andere Schläge 94 108 → −39,0 dBFS (eine Stimme: −31,5 dBFS); bei vollem MASTER ein Schlag 376 708 → −27,0 dBFS.
- **Aufnahme und Abspielen ohne Doppel:** Eine aufgenommene Note, die (nach QUANT) noch vor der Abspielposition
  liegt, hat schon live geklungen; sie wird in dieser Runde einmal übersprungen (Bit 7 der Velocity). Dasselbe gilt
  für eine Note kurz vor dem Ende des Vorzählers, die auf Tick 0 rückt (innerhalb einer halben QUANT-Stufe, bei OFF
  24 Ticks).

## Step-Sequencer (Stand 28.10.2026, nur Simulator)

- Kein eigener Speicher für Steps: Sie sind die Ereignisse des Loopers (Step = 1/16 = 48 Ticks, eine Note gehört zu
  dem Step, dessen Mitte sie auf ±24 Ticks trifft). `tools/device_check.py`: App 219 364 → 226 936 B, RAM
  55 200 → 55 328 B, Pool unverändert 110 344 B.
- Tastenbelegung: `PLAT_WHITE_KEYS` = {0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26} (`base/include/plat.h:28`),
  F3 = Taste 0; die Tasten haben Lichter (`plat_leds(buttons, keys)`, ebd.).
- Die Schriften enthalten nur ASCII bis Zeichen 126 (`base/tools/gen_font.py:126`): „·“ der Mockups ist auf dem Gerät
  „ - “.

## Effekte: Quellen und Speicher (Stand 28.10.2026, Recherche vor der Übernahme)

- **Mutable Instruments (github.com/pichenettes/eurorack, Commit `f4cfa76`, 12.10.2026):** README, Abschnitt
  „License“: „Code (STM32F projects): MIT license“, „Code (AVR projects): GPL3.0“. Rings, Clouds, Elements, Beads und
  Plaits sind STM32-Module; die Dateiköpfe tragen den MIT-Text („Copyright 2014 Emilie Gillet. Permission is hereby
  granted …“, z. B. `rings/dsp/fx/reverb.h`). Plaits nutzen wir schon für DRUMS (LICENSING.md).
- **Hall:** `rings/dsp/fx/reverb.h` und `clouds/dsp/fx/reverb.h` sind derselbe Aufbau: vier Allpässe zur Diffusion,
  dann zwei Schleifen mit je zwei Allpässen, Tiefpass und einer von einem LFO bewegten Verzögerung (Griesinger/
  Dattorro-Art). Gerechnet in `float`, gespeichert in 16 bzw. 12 Bit (`fx_engine.h`). Abhängigkeiten: `stmlib`
  (`Clip16`, `Interpolate`, `CosineOscillator`, MIT), ein Port nach C wie bei Plaits und Open303 nötig.

  | Quelle | Puffer | Speicher | Verzögerungen | gebaut für | bei 44,1 kHz |
  | --- | --- | --- | --- | --- | --- |
  | Rings | `FxEngine<32768, FORMAT_16_BIT>` | 65 536 B | 21 617 Samples | 48 kHz (`rings/dsp/dsp.h:39`) | Zeiten × 48 000 / 44 100 = × 1,088 |
  | Clouds | `FxEngine<16384, FORMAT_12_BIT>` | 32 768 B | 16 375 Samples | 32 kHz (`clouds/clouds.cc:115`) | Zeiten × 32 000 / 44 100 = × 0,726 (Raum kleiner); auf 44,1 kHz umgerechnet 22 567 Samples, passt nur in 32 768 |

- **Chorus:** `plaits/dsp/fx/ensemble.h` (`FxEngine<1024, FORMAT_32_BIT>`, 4 096 B) oder `rings/dsp/fx/chorus.h`
  (2 048 × 2 B). **Verzerrung:** `plaits/dsp/fx/overdrive.h` (ohne Puffer).
- **Delay:** einfach genug für eigenen Code (MPL-2.0), im Takt des Loopers. Speicher bei 44,1 kHz, 16 Bit, zwei Linien
  (Ping-Pong): 2 × Sekunden × 44 100 × 2 B.
- **Pool-Budget:** 344 064 − 110 344 (belegt mit Looper) − 8 192 (Reserve) = 225 528 B frei.

  | Delay je Seite | Delay | + Hall Rings (65 536) | bleibt | + Hall Clouds-Größe (32 768) | bleibt |
  | --- | --- | --- | --- | --- | --- |
  | 0,5 s | 88 200 B | 153 736 B | 71 792 B | 120 968 B | 104 560 B |
  | 0,75 s | 132 300 B | 197 836 B | 27 692 B | 165 068 B | 60 460 B |
  | 1,0 s | 176 400 B | 241 936 B | passt nicht | 209 168 B | 16 360 B |

  Ein Delay mit halber Rate (22,05 kHz, dunkler wie Band) bräuchte die Hälfte.
- **Rechenlast:** nicht gemessen. Der Hall rechnet je Sample etwa 14 Lese- und 12 Schreibzugriffe mit
  16-Bit-Umwandlung und rund 40 Gleitkomma-Operationen; wie viel das auf dem FM-1 ist, ist **unsicher**. Messen wie
  bei den Engines: erst relativ zu einer FM6-Stimme im Simulator, dann auf dem Gerät.

## Effekte: Umsetzung und Messungen (Stand 28.10.2026, nur Simulator)

- **Rings' Hall, wie Rings ihn belegt** (`rings/dsp/part.cc`, Stand `f4cfa76`, im Modell „String and reverb“):
  `set_diffusion(0.625f)`, `set_time(0.35f + 0.63f * damping)`, `set_input_gain(0.2f)`,
  `set_lp(0.3f + brightness * 0.6f)`, `set_amount(0.1f + damping * 0.5f)`. Übernommen: Diffusion und Eingang fest,
  DECAY 0–100 % = `set_time` 0,35 … 0,98, TONE 0–100 % = `set_lp` 0,3 … 0,9; `amount` = 1 (Send-Effekt: nur der Hall).
  Einstellbar sind in `reverb.h` nur `amount`, `input_gain`, `time`, `diffusion` und `lp`; die Längen der
  Verzögerungen stehen fest im Code (Reserve-Liste), daher kein SIZE. Der Hall nimmt `left + right` mono und gibt
  zwei Schleifen als links und rechts aus.
- **Speicher der Delay-Lines:** zehn Leitungen, jede ein Sample hinter der vorigen (`fx_engine.h`, `DelayLine::base`):
  150 + 214 + 319 + 527 + 2 182 + 2 690 + 4 501 + 2 525 + 2 197 + 6 312 = 21 617 Samples plus 10 = 21 627 ≤ 32 768
  (die Maske braucht eine Zweierpotenz, also 65 536 B).
- **Pool mit Effekten** (`tools/device_check.py`): 110 344 + 88 200 (Delay, 2 × 22 050 × 2 B) + 8 820 (PRE,
  4 410 × 2 B) + 65 536 (Hall) = 272 900 B von 344 064 B, gemessen 272 900 B; frei 71 164 B. RAM `.data` + `.bss`
  55 328 → 61 784 B (die Sends und Rückwege je Block: 2 × 256 × 4 + 2 × 512 × 4 = 6 144 B). App 226 936 → 235 452 B.
- **Rechenlast relativ zu FM6**, wie bei den Engines (Mac M3, `-O2 -ffp-contract=off`, 4 000 Blöcke zu 32 Frames,
  das Beste aus drei Läufen; eine FM6-Stimme = (6 Stimmen − 1 Stimme) / 5 = (8,84 − 1,68) / 5 = 1,43 ms):

  | Effekt | Zeit | relativ zu einer FM6-Stimme |
  | --- | --- | --- |
  | DELAY | 0,37 ms | 0,26 |
  | HALL (mit PRE) | 1,44 ms | 1,0 |
  | beide | 1,75 ms | 1,2 |

  Das ist ein Mac, nur das Verhältnis zählt; auf dem FM-1 **unsicher**. Ein Effekt, an den kein Part schickt, schläft,
  sobald sein Ausgang länger still war, als etwas in seinen Leitungen braucht (Delay 0,5 s, Hall 32 768 + 4 410
  Samples), und kostet dann nichts.
- **Pegel** (Host-Simulator, Part 1 SAW LEAD, MASTER 50 %, Send 100, LEVEL 0 dB, `tests/fx.txt`): das erste Echo
  223 038 gegen 223 166 der Note (gleich laut, −31,5 dBFS), nur links; das zweite 0,5 s später rechts 73 999
  (FDBK 35 % und TONE 70 % in der Rückkopplung). Der Hall 0,25–0,55 s nach dem Ende einer 600-ms-Note: −41 dBFS,
  etwa 10 dB unter der Note, links und rechts; bei DECAY 50 % fällt er im Test mit einem Impuls etwa 30 dB je Sekunde
  (RT60 also etwa 2 s, grob).
- **Delay-Zeit:** 44 100 × 60 / BPM × k / 24 = 110 250 × k / BPM Samples (k in 24steln einer Viertel: 1/32 = 3 …
  1/4 = 24); über 22 050 halbiert. Bei 40 BPM: 1/4 = 66 150 → 33 075 → 16 537 (1/16), 1/8. = 49 612 → 24 806 →
  12 403 (1/32.); mehr als zwei Halbierungen kommen bei 40–240 BPM nicht vor.

## LFO: was die Engines annehmen (Stand 29.10.2026, nur Simulator)

Quellen: `engines/src/felucca_engine.h` (`vmod_t`: Tonhöhe, Lautstärke-Rampe, `cutoff`, `shape`, `fine`, MIT),
`engines/src/eng_analog.c`, `eng_phase.c`, `eng_wheel.c`; KLASSIKs `dsp/eng_fm6.c`, `dsp/fm6/msfa.c`,
`dsp/eng_acid.c`, `dsp/acid/acid.c`.
- ANALOG (VA): `m->cutoff` ins Filter, `m->shape` als Pulsbreite (`analog_osc`, `pw`) bzw. bei SUB/SYNC als
  Tonhöhe von Oszillator 2 (`analog_render_x`); SAW, SQR, TRI, SIN ändern sich mit SHAPE nicht.
- PHASE (PD): `m->cutoff` und `m->shape` addieren sich beide zur DCW-Tiefe (`phase_loop`): ein Ziel, „DCW“.
- WHEEL (ORGAN): nur `m->inc` (Tonhöhe) und die Lautstärke; kein Filter, keine Form.
- FM6 (msfa): nur die Tonhöhe hat einen LFO-Eingang (`pitchmod`), keine Amplitudenmodulation; KLASSIK gibt eine
  Tonhöhen-Zugabe (Q24 je Oktave) und multipliziert den Block mit einer Rampe.
- ACID (Open303): keine Modulationseingänge; KLASSIK setzt je Block den Cutoff und einen Frequenzfaktor.

Messung (Simulator, 1/4 bei 120 BPM = 2 Hz, Tiefe 100, gehaltene C4, 25-ms-Fenster; Schwankung = größter / kleinster
Wert, ohne LFO → mit LFO): VA PITCH Nulldurchgänge 2,1 → 5,2, FILTER Pegel 1,8 → 10,2, AMP Pegel 1,8 → 190, SHAPE
(SAW) unverändert, SHAPE mit SYNC Helligkeit 1,08 → 1,57; PD DCW Helligkeit 1,05 → 1,95, PITCH Nulldurchgänge
1,1 → 4,3; ORGAN PITCH 1,3 → 4,6, AMP Pegel 1,4 → 263; FM6 PITCH 1,2 → 4,3, AMP Pegel 2,1 → 921; ACID FILTER
Nulldurchgänge 4,4 → 12,6, PITCH Periode bis 86 statt 168 Samples (eine Oktave). Helligkeit = Effektivwert der
ersten Differenz durch den Effektivwert. Nach Gehör noch nicht geprüft.

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
