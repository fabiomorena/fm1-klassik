# Sechs Parts – Testprotokoll (Browser-Emulator)

Stand v0.6 (Teil sechs Parts). Erst ausfüllen, wenn die Protokolle `PARTS.md`, `MIXER.md`, `LOOPER.md`, `STEP.md` und
`FX.md` fertig sind: Für dieses Protokoll baue ich den Emulator neu, danach gelten dort die Stellen mit vier Parts
nicht mehr. Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/app.c`) und den Host-Tests
(`firmware/klassik/tests/parts.txt`, `fx.txt`). Es geht um Funktion und Stabilität.

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben. HOME im Emulator: `Esc`; Taster
festhalten: Shift-Klick.

## 0 Vorbereitung

- [ ] 0.1 `http://localhost:8000` mit Cmd+Shift+R neu laden, **ohne** „Factory reset“, dann „Switch on“. Was du
  vorher gespeichert hattest (Parts 1–4, Mixer, Sends, Effekte), ist noch da; Part 5 spielt PD, Part 6 ORGAN.

## A Parts

- [ ] A1 Im Rack: jede Kachel hat eine Part-Nummer (FM6 4, VA 1, PD 5, ORGAN 6, DRUMS 2, ACID 3; falls du vorher
  Engines getauscht und gespeichert hattest, entsprechend).
- [ ] A2 SEL: 1, 2, 3, 4, 5, 6, 1. Die Kopfzeile zeigt Part und Engine (`5 PD`, `6 ORGAN`). SELECT nach links geht
  von 1 auf 6.
- [ ] A3 Auf Part 5 und 6 spielen, Engine (K1), Sound (PRESET), Oktave und Edit-Seiten wie bei den anderen Parts.
- [ ] A4 MIDI (falls du einen Controller oder eine DAW hast): Kanal 5 spielt Part 5, Kanal 6 Part 6.

## B Mixer

- [ ] B1 GLO bei Part 1–3: Kopfzeile `PARTS 1-3`, drei Kanalzüge plus MASTER, rechts in der Tab-Zeile die linke
  Marke hell. Die Züge sind breiter als vorher, die Namen ganz lesbar.
- [ ] B2 SEL bis Part 4: die Seite wechselt zu `PARTS 4-6` (4 FM6, 5 PD, 6 ORGAN), die rechte Marke ist hell. SEL
  von 6 auf 1: zurück zur ersten Seite.
- [ ] B3 LEVEL, PAN, MUTE, SOLO und auf `SEND` die Sends wirken auf Part 5 und 6 wie auf die anderen. SOLO auf
  Part 5: alle anderen Parts, auch die auf der anderen Seite, sind still.

## C Looper und Step

- [ ] C1 Looper-Seite (SEQ): sechs Spuren untereinander, je Glyphe und Name in einer Zeile. Auf Part 5 und 6
  aufnehmen: die Noten erscheinen in ihren Spuren; ein roter Punkt vor der Spur, die aufnimmt.
- [ ] C2 Die Spuren sind flacher als vorher. Sind Noten, vor allem DRUMS, noch gut zu erkennen? (bitte notieren)
- [ ] C3 Tab STEP: sechs Reihen; auf Part 5 und 6 Steps setzen (weiße Tasten), sie spielen. Unten die Angaben zum
  Step (`NOTE`, `VEL`, `LEN` nebeneinander).

## D Effekte

- [ ] D1 FX-Seite: sechs Kacheln (drei nebeneinander, zwei Reihen) mit den Sends aller Parts.
- [ ] D2 Part 6 (ORGAN) mit HALL, Part 5 (PD) mit DELAY: beide Effekte klingen.

## E Zusammen und Stabilität

- [ ] E1 Alle sechs Parts zugleich spielen (Looper mit sechs Spuren oder MIDI): nichts bricht ab, keine Aussetzer.
  Werden es zu viele Stimmen, endet die älteste (wie bisher).
- [ ] E2 SAVE, Seite neu laden, „Switch on“: alle sechs Parts mit Engine, Sound, Oktave und Mixer wie vorher.
- [ ] E3 Fünf Minuten spielen, Parts wechseln, Seiten wechseln: kein Absturz.
