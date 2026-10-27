# DRUMS – Testprotokoll (Browser-Emulator)

Stand v0.3 (Teil DRUMS). Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/dsp/eng_drums.c`,
`firmware/klassik/dsp/engines.c`, `firmware/klassik/app.c`) und den Host-Tests (`firmware/klassik/tests/drums.txt`).
Wo die Erwartung unsicher ist, steht „(unsicher)“: Dann einfach notieren, was passiert.

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

In den Tabellen ersetzt du `·` durch `x`, `!` oder `-`; zu jedem `!` eine Notiz unter der Tabelle.
Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

Im Browser nicht prüfbar: der Accent (jede Taste kommt mit Velocity 100, `base/web/emu/emu_web.c:86`), MIDI-In,
Rechenlast und Pegel auf dem Gerät.

## 0 Vorbereitung

| Was | Wie im Emulator |
| --- | --- |
| Regler drehen | Mausrad über dem Regler, oder Regler anklicken und Pfeiltasten (Bild↑ / Bild↓: 5 Schritte) |
| Taste festhalten | Rechtsklick oder Shift-Klick: bleibt gedrückt, bis du noch einmal klickst |
| weiße Tasten | `Q W E R T Z U I` = F3 … F4, `A S D F G H J K` = G4 … G5 (`T` = C4) |
| schwarze Tasten | `1` … `0` und `ß` = F#3 … F#5 |
| OCT− / OCT+ | `Y` / `X` |

Die Instrumente auf der Computertastatur (Oktave +0):

| Taste | `Q` | `1` | `W` | `2` | `E` | `3` | `R` | `T` | `4` | `Z` | `5` | `U` |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Note | F3 | F#3 | G3 | G#3 | A3 | A#3 | B3 | C4 | C#4 | D4 | D#4 | E4 |
| Instrument | LT | CH | MT | CH (seit 27.10.: je Kit) | HT | OH | CY | BD | RS | SD | CP | CB |

Die Kürzel wirken nur, wenn das Panel den Fokus hat: einmal aufs Gerät klicken.

- [ x] 0.1 Der Server auf `http://localhost:8000` läuft und zeigt den neuen Stand (von Claude gebaut). Falls die
  Seite nicht lädt, im Terminal: `python3 -m http.server 8000 -d firmware/klassik/build/emu`; falls DRUMS fehlt:
  `.venv/bin/python base/tools/build.py firmware/klassik --web` (letzte Zeile `emu … (… B wasm)`).
- [ x] 0.2 `http://localhost:8000` mit Cmd+Shift+R neu laden, „Factory reset“ zweimal, dann „Switch on“.
  Erwartet: Status „On.“; Rack mit VA umrandet. (Die Einstellungen von v0.2 sind dabei ohnehin verworfen: neues
  Speicherformat.)

## A Rack

- [ x] A1 Die fünfte Kachel heißt DRUMS (lila, „DRUM KITS“, Glyphe, „606“); ACID zeigt weiter „SOON“.
- [ x] A2 K1 von VA aus drei Rasten nach rechts: DRUMS ist gewählt, Kopfzeile lila Chip `DRUMS`, Glyphe, `606`;
  Fußzeile `K1 ENGINE DRUMS`, `PRESET 01/04`. Eine Raste weiter: wieder FM6.
- [ x] A3 PRESET durch alle vier Kits: 606, 707, 808, 909, dann wieder 606; jedes hat eine eigene Glyphe.

## B Tasten und Instrumente

- [ !] B1 Mit 808 jede Taste der Tabelle oben einmal: jede spielt das Instrument der Tabelle.
- [ !] B2 Dieselbe Belegung in jeder Oktave: mit `Y` / `X` die Oktave wechseln, `T` spielt immer BD, `Z` immer SD.
- [ x] B3 `R` (CY) halten oder nicht: Jeder Schlag klingt von selbst aus, das Loslassen ändert nichts.
- [ x] B4 OH (`3`) anschlagen und, während sie klingt, CH (`1`): Die offene Hi-Hat verstummt sofort.
- [ x] B5 Dasselbe Instrument schnell wiederholt (z. B. `T` BD): Jeder Schlag ist zu hören, kein Knacken (unsicher).
- [x ] B6 CH auf `1` und auf `2` abwechselnd schnell: klingt wie eine Hi-Hat (eine Stimme), kein Knacken.
- [ x] B7 Schnell durcheinander spielen (mehr als sechs Instrumente gleichzeitig klingend): nichts hängt, kein Absturz.

## C Kits nach Gehör

Je Kit alle Instrumente anspielen und einen einfachen Beat (BD `T`, SD `Z`, CH `1`). In die Spalten `x`, `!` oder
`-`. Zum Vergleich ohne Browser: `firmware/klassik/build/host/drums-606.wav` … `drums-909.wav` (nach den Host-Tests).

| Kit | gedacht als | Charakter passt | BD | SD | Toms | Hats | CP / RS / CB | CY | Lautstärken ausgewogen |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 606 | klein, hoch, kurz, analog | · | · | · | · | · | · | · | · |
| 707 | 909-ähnlich, „gesampelt“, etwas körnige Hats | · | · | · | · | · | · | · | · |
| 808 | lange, dröhnende BD, weiche Toms, typische 808-Hats | · | · | · | · | · | · | · | · |
| 909 | kräftige BD mit Pitch-Sweep, körnige Hats und Becken | · | · | · | · | · | · | · | · |

Notizen:

B1: es gibt 2 mal closed hi-hat 
B2: oktave +/- verändert den Sound in den Drums nicht / low- mid and hig tom hört sich eher nach einer bassdrum an
B6: Open Hihat und Cymbal hören sich sehr blechern an


## D Edit-Seite DRUM

Mit dem Kit 808 beginnen.

- [x ] D1 EDIT: Seite `DRUM` (`1/2`), darunter „KEYS“ und „EVERY OCTAVE“, die zwölf Pads wie die Tasten einer Oktave
  (oben RS, CP, CH, CH, OH; unten BD, SD, CB, LT, MT, HT, CY); BD und SD lila umrandet. Fußzeile
  `K1 BD TUNE 0`, `K2 BD DEC 70`, `K3 SD TUNE 0`, `K4 SNAPPY 45`.
- [ x] D2 Eine Taste spielen: Ihr Pad leuchtet kurz lila auf (etwa eine Viertelsekunde).
- [x ] D3 BD TUNE −12 … +12: die Bassdrum tiefer / höher, bei 0 wie vorher; die Grenzen halten.
- [ x] D4 BD DEC 0 … 99: 0 kurzer Schlag, 99 langes Dröhnen.
- [ !] D5 SD TUNE −12 … +12: die Snare tiefer / höher.
- [x ] D6 SNAPPY 0 … 99: 0 fast nur der Kessel (Ton), 99 fast nur das Rauschen des Teppichs.
- [ x] D7 Nach der ersten Änderung: `808*` in der Kopfzeile, SAVE leuchtet; die Glyphe in der Kopfzeile ändert sich.
- [x ] D8 Alle vier wirken sofort, auch auf einen gerade klingenden Schlag (eine lang klingende Bassdrum ändert beim
  Drehen von BD TUNE hörbar ihre Tonhöhe).

## E Edit-Seite KIT

- [ x] E1 EDIT noch einmal (oder OCT+): Seite `KIT` (`2/2`); umrandet sind jetzt CH, CH, OH, LT, MT, HT. Fußzeile
  `K1 TOM TUNE 0`, `K2 HAT DEC 30`, `K3 HAT TONE 50`, `K4 DRIVE 0`.
- [ !] E2 TOM TUNE: alle drei Toms gemeinsam tiefer / höher.
- [ !] E3 HAT DEC: geschlossene und offene Hi-Hat kürzer / länger (die geschlossene bleibt kurz).
- [! ] E4 HAT TONE: Hi-Hats dunkler / heller; das Becken (`R`) ändert sich nicht.
- [x ] E5 DRIVE 0 … 99: alle Instrumente kräftiger, bei 99 deutlich verzerrt.
- [x ] E6 OCT− zurück ins Rack; PRESET auf einer Edit-Seite lädt das nächste Kit, das `*` ist weg.

## F Extremwerte je Kit

Je Kit jeden Parameter einmal auf Minimum und Maximum, dann BD, SD, eine Tom und die Hats anspielen. Gesucht:
Stille, Knacken, Verzerrung (außer bei DRIVE), hängende Töne, Abstürze. Zwischen zwei Parametern das Kit mit PRESET
neu laden.

| Kit | BD TUNE | BD DEC | SD TUNE | SNAPPY | TOM TUNE | HAT DEC | HAT TONE | DRIVE |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 606 | · | · | · | · | · | · | · | · |
| 707 | · | · | · | · | · | · | · | · |
| 808 | · | · | · | · | · | · | · | · |
| 909 | · | · | · | · | · | · | · | · |

Notizen:

D5: Snare Drumm ändert sich nicht bei Tune und Dec
E2: Tune ändert nichts am Sound


## G Speichern

- [ x] G1 Ein Kit ändern, SAVE: kurz `SAVED USER`, das `*` ist weg; im Rack `PRESET USER`.
- [x ] G2 PRESET: nach 909 kommt USER, danach 606.
- [ x] G3 Seite neu laden, „Switch on“: DRUMS ist gewählt, `USER` mit deinen Werten.
- [ x] G4 Die anderen Engines (FM6, VA, PD, ORGAN) spielen wie vorher (kurz anspielen).

## H Dauerbetrieb

- [ ] H1 Fünf Minuten Beats spielen, dabei Kits und Regler wechseln: keine Aussetzer, kein Knistern, kein Absturz.
  (Der Browser rechnet anders als das Gerät; Aussetzer hier sagen wenig über das Gerät.)

## Freie Notizen

Was dir sonst auffällt: Klang, Bedienung, Anzeige, Wünsche.
Ansonsten erst einmal alles in Ordnung

## Auswertung (Claude, 27.10.2026)

- Ohne Befund: Rack, Speichern, Choke, BD TUNE/DEC, SNAPPY, DRIVE, die anderen Engines. Tabellen C und F und H1
  nicht ausgefüllt; laut Notiz sonst alles in Ordnung.
- B1 „zweimal closed hi-hat“: war so gebaut (G# wie in General MIDI die Pedal-Hi-Hat). Jetzt hat G# je Kit ein
  eigenes Instrument: 606 und 909 RD (Ride), 707 TB (Tamburin), 808 CL (Claves); Fabios Entscheidung.
- B2 „Oktave ändert nichts“: so gebaut, bleibt so (Fabios Entscheidung). „Toms klingen nach Bassdrum“: stimmte, sie
  nutzten das Bassdrum-Modell. Jetzt ein eigenes Tom-Modell, das nach dem Anschlag in der Tonhöhe fällt.
- B6 „OH und Becken blechern“, E4 HAT TONE: Das Filter der Hats lag zu tief (OH 1,2 kHz, Becken 0,6 kHz). HAT TONE
  deckt jetzt 1,9–9,6 kHz ab, die Becken sind heller.
- D5 / E2 „SD TUNE / TOM TUNE ändern nichts“: Im Simulator verschieben beide um je eine Oktave (Messung in
  `docs/RESEARCH.md`). Zum Anhören, am besten über Kopfhörer: `firmware/klassik/build/host/drums-tune.wav` (Snare
  bei −12, 0, +12, dann die tiefe Tom bei −12, 0, +12). Einen Snare-Ausklang (SD DEC) gibt es nicht (Fabios
  Entscheidung).
- E3 HAT DEC: `!` ohne Notiz; gemessen wirkt er (offene Hi-Hat 284 ms bei 0, über 1,4 s bei 99). Was war auffällig?

Nachtest (nach dem Commit der Korrekturen):

- [ ] N1 G# spielt je Kit RD, TB, CL, RD; das Pad zeigt den Namen.
- [ ] N2 Die Toms klingen nach Toms (setzen höher ein und fallen), nicht nach Bassdrum.
- [ ] N3 OH und Becken nicht mehr blechern; HAT TONE macht die Hats hörbar dunkler / heller.
- [ ] N4 SD TUNE und TOM TUNE über Kopfhörer: hörbar tiefer / höher (auch `drums-tune.wav`).
