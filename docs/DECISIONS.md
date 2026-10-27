# Entscheidungen

Neueste unten. Format: Datum · Entscheidung · Begründung · Status.

| Datum | Entscheidung | Begründung | Status |
| --- | --- | --- | --- |
| 25.10. | Ziel: eigene FM-1-Firmware mit mehreren klassischen Synth-Engines, eigenständige, übersichtliche UI | Fabios Vorgabe | fest |
| 25.10. | Basis **fm1-base** statt Felucca-Fork oder fm1-dock | MPL-2.0 (freie Lizenzwahl), Simulator, sauberer Start für eigene UI | von Claude empfohlen, mit dem Repo-Gerüst übernommen |
| 25.10. | Keine GPL-Quellen; Engines aus Felucca-Engines (MIT), später msfa (Apache-2.0) und Open303 (MIT) | Lizenz bleibt frei wählbar | fest |
| 26.10. | Eigene Dateien unter **MPL-2.0** | gleiche Lizenz wie fm1-base | Vorschlag von Claude, von Fabio noch nicht ausdrücklich bestätigt |
| 26.10. | Display **240×240** | mehrere Quellen, siehe RESEARCH.md; erste Mockups (320×240) verworfen | fest |
| 26.10. | **Glyphe** als durchgehendes Erkennungszeichen jedes Sounds; Raster Kopfzeile / Inhalt / Reglerzeile | Signatur der UI | fest |
| 26.10. | Signatur-Ideen Klang-Atlas, Glyphen-Browser, Klang-Stammbaum | in Verzeichnissen nicht gefunden (Neuheit unsicher) | geplant (v0.4–v0.6) |
| 26.10. | v0: VA (ANALOG), PD (PHASE), ORGAN (WHEEL); 1 Part, 6 Stimmen | schnell spielbar, MIT | umgesetzt, im Simulator getestet |
| 26.10. | Sequencer und Mixer mit 4 Parts | Annahme für die Mockups (Stock V16 hat 8) | offen |
| 26.10. | Repo `fabiomorena/fm1-klassik`, CI mit Host-Tests | Sicherung und automatische Prüfung | umgesetzt, CI grün |
| 26.10. | Arbeit nur noch in PyCharm mit Claude Code; `docs/design/` ersetzt das Canvas | Fabios Wunsch | fest |
| 26.10. | v0.1: EDIT mit zwei Seiten à vier Parametern auf K1–K4, Live-Glyphe, ein USER-Sound je Engine (SAVE); Hüllkurve, Pegel und Regler-Beschleunigung erst mit Gerät | acht Parameter je Engine (`engine_t.edit[8]`, `page_title[2]`) passen auf K1–K4; Feinabstimmung braucht Hardware | umgesetzt, im Simulator getestet (Commit `aa1433c`) |
| 26.10. | Edit-Seiten: das Mockup `Edit-VA` (fünf Seiten, Filterkurve) bleibt Ziel; v0.1 (zwei Seiten, große Glyphe) ist eine Zwischenstufe | Fabios Entscheidung | fest |
| 26.10. | FM6 auf msfa aus dem Original-Repo von Google (Apache-2.0), nach C portiert; Tabellen beim Build (Python) | der Build ist C99 ohne `double` (`base/tools/build.py`), Dexed ist GPL-3.0 | umgesetzt, im Simulator getestet; gegen das Original geprüft (RESEARCH.md) |
| 26.10. | FM6-Presets: acht eigene Klänge, keine DX7-Werksklänge | Rechte an den Werksklängen bei Yamaha, für eine freie Firmware ungeklärt | Fabios Entscheidung, fest; Klang nach Gehör noch zu prüfen |
| 26.10. | FM6-Parameter als absolute Makros: ALGO, FEEDBK, MOD, DTUN / ATK, DEC, SUS, REL; ein Preset ist eine DX7-Stimme plus die acht Werte, die sie beschreiben, gedreht wird relativ dazu | Glyphen der Presets unterscheiden sich; die Stimme behält ihren Charakter | Fabios Entscheidung, fest; Bearbeitung je Operator (Mockup `Edit-FM6`) bleibt Ziel |
| 26.10. | Keine 64-Bit-Multiplikation inline in DSP-Schleifen; FM6-Gain in 32 Bit, `app_render` über `noinline`-Funktion | `base/bsp/docs/CPU.md` (Rechenfehler neben Ladebefehlen) | umgesetzt; Genauigkeit gemessen (RESEARCH.md) |
| 26.10. | K1 wählt die Engines in der Reihenfolge des Racks (FM6, VA, PD, ORGAN); die Engine-Nummern bleiben (FM6 = 3) | Bedienung folgt dem Bildschirm; Tests und Speicherformat bleiben lesbar | umgesetzt |
| 26.10. | Speicherformat `KLS3`: Einstellungen von v0.1 werden verworfen | ein USER-Sound mehr; noch kein Gerät im Einsatz | umgesetzt |
| 26.10. | Die CI baut bei jedem Push auch den Browser-Emulator (`web.yml`, Emscripten 6.0.13 aus emsdk) | ein Namenskonflikt mit `emu_web.c` (seit v0.1) fiel nur im Web-Build auf | Fabios Entscheidung, umgesetzt |
| 26.10. | Manuelle Tests als Protokoll zum Abhaken in `docs/tests/` (erstes: `FM6.md`) | Fabio testet im Browser und gibt die Ergebnisse gesammelt zurück | Fabios Wunsch (Ort von Claude gewählt); FM6 getestet, alles wie erwartet bis auf Knacken bei sehr schnellem Spiel |
| 26.10. | FM6: Ein Wiederanschlag einer noch klingenden Stimme macht je Operator dort weiter, wo er ist (über −42 dB); leisere Operatoren starten frisch (`fm_note_retrigger` in `msfa.c`) | msfas Note-Start setzt Phase und Hüllkurve auf 0: Knacken (Messung in RESEARCH.md) | Fabios Entscheidung, umgesetzt, im Simulator gemessen; im Browser von Fabio angehört: „viel besser“ |
| 26.10. | v0.3: **DRUMS ersetzt STRINGS** im Rack (FM6, VA, PD / ORGAN, DRUMS, ACID); DRUMS übernimmt die Farbe von STRINGS (185,162,255) | Fabio wünscht Drums; STRINGS gab es noch nicht und hatte keine eigene Quelle (nur ANALOG + Chorus), ORGAN ist fertig und getestet | Fabios Entscheidung (Claudes Empfehlung), fest |
| 26.10. | DRUMS: vier Kits im Stil von TR-606, TR-707, TR-808, TR-909, auf dem Gerät „606“, „707“, „808“, „909“ genannt | für Spieler am klarsten | Fabios Entscheidung; markenrechtlich ungeklärt (offene Punkte) |
| 26.10. | DRUMS-Klänge synthetisch: Plaits-Drums (MIT) nach C portiert und eigener Code; keine Original-Samples | 707 und die Hats/Becken der 909 sind Samples, Rechte bei Roland (RESEARCH.md) | umgesetzt; Port gegen Plaits geprüft (RESEARCH.md) |
| 26.10. | DRUMS-Tasten je Halbton, in jeder Oktave gleich: C BD, C# RS, D SD, D# CP, E CB, F LT, F# CH, G MT, G# CH (seit 27.10.: je Kit), A HT, A# OH, B CY (BD, RS, SD, CP, CH, OH wie General MIDI); Anschlag = Accent; CH beendet OH | spielbar auf jeder Oktave; ein GM-Sequencer trifft die wichtigsten Instrumente | Mockup `Edit-DRUMS` von Fabio angenommen; umgesetzt, im Simulator getestet |
| 26.10. | Reihenfolge v0.3: erst DRUMS, dann ACID (Open303, MIT), je mit Mockup, Testprotokoll und eigenem Commit | Fabios Wunsch nach Drums zuerst | DRUMS umgesetzt, ACID offen |
| 26.10. | DRUMS-Edit-Seite: die Tasten einer Oktave als Pads statt der großen Glyphe (Glyphe bleibt in der Kopfzeile); die Instrumente der Seite umrandet, das zuletzt gespielte leuchtet 250 ms | ohne Anzeige weiß man nicht, welche Taste was spielt | Fabio hat das Mockup angenommen; umgesetzt |
| 26.10. | DRUMS: eine Stimme je Instrument (wie bei den Originalen), ein neuer Schlag klingt vom aktuellen Zustand weiter; CH beendet OH in ~5 ms; eine Stimme endet, wenn sie 8 Blöcke unter ~−84 dBFS (bei vollem MASTER) war | Spielgefühl der Vorbilder; Stimmen frei für andere Instrumente | umgesetzt, im Simulator getestet |
| 26.10. | Speicherformat `KLS4`: Einstellungen von v0.2 werden verworfen | eine Engine mehr (DRUMS) | umgesetzt |
| 27.10. | DRUMS G#: ein eigenes Instrument je Kit statt einer zweiten CH: 606 RD (Ride), 707 TB (Tamburin), 808 CL (Claves), 909 RD | Fabios Test: „zweimal closed hi-hat“; was die Maschinen noch hatten | Fabios Entscheidung (Claudes Empfehlung); umgesetzt, Mockup angepasst |
| 27.10. | DRUMS: die Oktave ändert nichts am Klang; kein SD DEC | GM-Belegung in jeder Oktave; die Originale hatten keinen Snare-Decay | Fabios Entscheidung (Claudes Empfehlung) |
| 27.10. | DRUMS-Toms: eigenes Modell (Sinus mit Tonhöhenabfall, Stockgeräusch) statt des Bassdrum-Modells; HAT TONE 1,9–9,6 kHz statt 0,15–9,6 kHz, hellere Becken | Fabios Test: Toms „nach Bassdrum“, OH und Becken „blechern“ (Messung in RESEARCH.md) | umgesetzt, im Simulator gemessen; nach Gehör noch zu prüfen |
| 27.10. | ACID-Makros: TUNE, CUTOFF, RESO, ENV MOD (EDIT 1), DECAY, ACCENT, WAVE, SLIDE (EDIT 2): die sieben Regler der TB-303 plus Slide | Spieler kennen sie; Slide ist bei der 303 fest, bei Open303 einstellbar | Fabios Entscheidung (Claudes Vorschlag); Seiten FILTER / VOICE, Mockup `Edit-ACID` von Fabio angenommen |
| 27.10. | ACID-Slide über Legato: eine Taste halten und die nächste drücken, dann gleitet der Ton; die letzte losgelassen, gleitet er zur noch gehaltenen zurück | so macht es Open303 (`noteOn`, `releaseNote`); ein Sequencer kommt erst mit v0.5 | Fabio hat den Plan angenommen |
| 27.10. | ACID-Accent über einen Taster, der beim Anschlag gehalten wird; MIDI-Velocity ≥ 100 wie bei Open303 | Anschlagstärke liefern nur die Tasten auf den analogen Reihen (`plat_sense`), ohne Gerät nicht prüfbar | Fabios Entscheidung: Taster ENV (auf ACID ohne andere Aufgabe), `ACC` in der Kopfzeile, solange er gehalten ist (Mockup angenommen) |
| 27.10. | ACID-Port: Open303 nach C mit float; Wellenform-Tabellen beim Build (int16, Tabellen 0–9, ab Tabelle 5 halb so lang: 48 936 B statt 393 984 B), das elliptische Filter als sechs Biquads | kein double auf dem FM-1; die Direktform 12. Ordnung hält in float nicht (RESEARCH.md) | umgesetzt, gegen das Original geprüft (56–67 dB, RESEARCH.md) |
| 27.10. | ACID-Filter: der Modus `TB_303` des Kerns statt `LP_18`, den das Open303-Plugin als Standard nimmt | der Kern ist „die 303“ des Autors; beide sind portiert | Fabios Entscheidung nach Gehör (`acid-tb303.wav` gegen `acid-lp18.wav`) |
| 27.10. | ACID im Host: MONO-Presets merken sich die gehaltenen Tasten (bis 8); ACID ist die sechste Engine (`KL_ACID` = 5), K1 erreicht sie nach DRUMS; Speicherformat `KLS5` (Einstellungen von v0.3 Teil DRUMS werden verworfen) | Slide und Zurückgleiten brauchen die gehaltenen Tasten; eine Engine mehr | umgesetzt, im Simulator getestet (`tests/acid.txt`) |
| 27.10. | ACID: TUNE wirkt sofort, ein gehaltener Ton gleitet dorthin (Open303 stimmt erst die nächste Note) | ein Stimm-Regler, der erst bei der nächsten Note wirkt, fühlt sich kaputt an | Claudes Entscheidung, im Protokoll C5 zu prüfen |
| 27.10. | ACID-Pegel `AC_LEVEL` = 2 × VOICE_FS: normale Noten −27 … −33, Accents −16 … −25 dBFS bei MASTER 50 % (F3); hohe Lagen mit viel Resonanz und Accent übersteuern bei vollem MASTER (bis +5,6 dB) | so laut wie die anderen Engines; ein Limiter wäre eine Abweichung von Open303, sechs VA-Stimmen übersteuern dort auch | Claudes Vorschlag; Feinabstimmung mit Gerät, Fabio prüft F2 im Protokoll |

## Offene Punkte

- `id_prefix = "FM-1_77"` vor dem ersten Release auf Kollisionen prüfen.
- Ausgangspegel (×32) auf Hardware einmessen.
- Geräte-Build (Docker, JieLi-Toolchain) einrichten und im fm1-emu testen.
- Beim Kauf des Geräts: ausgelieferte Firmware-Version prüfen (Werkzeuge sind auf V15 ausgelegt).
- Lizenz MPL-2.0 bestätigen oder ändern.
- Kit-Namen „606“, „707“, „808“, „909“ vor dem ersten Release markenrechtlich prüfen (DPMA, USPTO; Roland hat 2019
  Designs von TR-808 und TB-303 angemeldet, RESEARCH.md).
- DRUMS nach Gehör prüfen (Protokoll `docs/tests/DRUMS.md`, `drums-*.wav`) und die Kits nachstimmen.
- DRUMS-Rechenlast auf dem Gerät messen (Schätzung: 10–22 % von 360 MHz bei sechs Snare-Stimmen, unsicher).
- FM6-Rechenlast auf dem Gerät messen (Schätzung: 7–13 % von 360 MHz bei sechs Stimmen, unsicher).
- ACID-Rechenlast auf dem Gerät messen (Schätzung: 12–18 % von 360 MHz für die eine Stimme, unsicher).
- FM6-Edit-Seite nach dem Mockup `Edit-FM6` (Algorithmus-Grafik, Pegel je Operator).
