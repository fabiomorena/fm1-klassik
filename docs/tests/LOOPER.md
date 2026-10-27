# Looper – Testprotokoll (Browser-Emulator)

Stand v0.5 (Teil Looper). Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/app.c`) und dem
Host-Test (`firmware/klassik/tests/looper.txt`). Es geht um Funktion und Stabilität; Klang und Design kommen zum
Schluss.

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

Im Browser nicht prüfbar: Aufnahme über MIDI-In (der Emulator hat kein Web-MIDI; das prüft der Host-Test), wie genau
die Noten auf dem Gerät sitzen, die Rechenlast.

## 0 Vorbereitung

| Was | Wie im Emulator |
| --- | --- |
| Regler drehen | Mausrad über dem Regler, oder Regler anklicken und Pfeiltasten (Bild↑ / Bild↓: 5 Schritte) |
| Taste oder Taster festhalten | Rechtsklick oder Shift-Klick: bleibt gedrückt, bis du noch einmal klickst |
| weiße Tasten | `Q W E R T Z U I` = F3 … F4 (`T` = C4) |
| PLAY, REC, HOME | `P`, `O`, `Esc` |
| HOME + REC | `Esc` festhalten und `O` drücken (oder HOME mit Shift-Klick festhalten, REC klicken, HOME wieder klicken) |
| OCT− / OCT+ | `Y` / `X` |
| SEL, SELECT | SEL anklicken; SELECT: Pfeiltasten ↑ / ↓ |

Die Tastatur-Kürzel wirken nur, wenn das Panel den Fokus hat: einmal aufs Gerät klicken.

- [ ] 0.1 `http://localhost:8000` mit Cmd+Shift+R neu laden, „Switch on“. (Falls die Seite nicht lädt, im Terminal:
  `python3 -m http.server 8000 -d firmware/klassik/build/emu`)

## A Seite und Regler

- [ ] A1 SEQ: Kopfzeile `LOOP EMPTY`, SEQ leuchtet; vier Spuren `1 VA`, `2 DRUMS`, `3 ACID`, `4 FM6`, jede `EMPTY`;
  Fußzeile `K1 TEMPO 120 BPM`, `K2 LENGTH AUTO`, `K3 QUANT 1/16`, `K4 CLICK REC`.
- [ ] A2 PLAY auf der leeren Schleife: Meldung `EMPTY REC FIRST`, nichts startet.
- [ ] A3 K1 drehen: das Tempo ändert sich (40 … 240), im Balken erscheint eine helle Marke bei 120. HOME halten und K1
  drehen: zurück auf 120, die Marke ist weg. Das Gleiche mit K3 (Marke bei 1/16) und K4 (Marke bei REC).
- [ ] A4 OCT− schließt die Seite (zurück zum Rack), SEQ öffnet sie wieder; GLO und EDIT wechseln zu Mixer und
  Edit-Seite.

## B Die erste Aufnahme

- [ ] B1 Part 1 (VA) gewählt, gestoppt, REC: Kopfzeile `LOOP COUNT-IN`, rechts ein roter Punkt und `4`, `3`, `2`, `1`;
  vier Klicks (der erste höher). REC und PLAY leuchten (PLAY rot). An der Spur 1 steht `REC`.
- [ ] B2 Nach dem Vorzähler: `REC 1 VA`, rechts roter Punkt und Takt.Schlag (`1.1`, `1.2`, …). Zwei Takte lang
  etwas spielen, kurz nach Beginn von Takt 3 REC: `PLAY`, die Länge ist 2 Takte (`K2 LENGTH 2 BARS`), rechts ein
  grünes Dreieck. Die Schleife wiederholt, was du gespielt hast; die Noten erscheinen als Striche in Spur 1.
- [ ] B3 Die Noten sitzen auf dem 1/16-Raster (leicht ungenau gespielt klingt es trotzdem gerade).
- [ ] B4 Beim Aufnehmen klickt es auf jedem Schlag, nach REC nicht mehr (CLICK REC).
- [ ] B5 REC während des Vorzählers bricht ab: gestoppt, nichts aufgenommen, LENGTH bleibt AUTO.

## C Overdub, Zurücknehmen, Leeren

- [ ] C1 Während die Schleife spielt: SEL zu Part 2 (DRUMS), REC (ohne Vorzähler), Drums dazuspielen, REC. Beide
  Spuren spielen zusammen weiter.
- [ ] C2 HOME + REC: Meldung `UNDO 2 DRUMS`, die Drums sind weg; noch einmal HOME + REC: `REDO 2 DRUMS`, wieder da.
- [ ] C3 Auf Part 1 wechseln, HOME + PLAY: Meldung `CLEAR 1 VA? OCT+`. OCT−: `KEPT 1 VA`, nichts geändert (die Oktave
  auch nicht). Noch einmal HOME + PLAY, dann OCT+: `CLEARED 1 VA`, Spur 1 leer, die Drums spielen weiter.
  HOME + REC: Spur 1 wieder da.
- [ ] C4 REC auf Part 1, dann während der Aufnahme SEL: `REC 2 DRUMS`, die Aufnahme ist mit zum nächsten Part
  gewandert.
- [ ] C5 Während die Schleife spielt, eine andere Note festhalten: Sie klingt weiter, bis du loslässt. (Spielt die
  Schleife genau die Note, die du hältst, beendet ihr Ende auch deine; das ist bekannt.)

## D Länge, Takt, Kopfzeile

- [ ] D1 K2 LENGTH auf 4: die Spuren zeigen 4 Takte, die Takte 3 und 4 sind leer; die Marke im Balken steht bei 2.
  HOME + K2: zurück auf 2. Auf 1: nur der erste Takt spielt, Takt 2 ist ausgeblendet; zurück auf 2: wieder da.
- [ ] D2 K1 TEMPO während des Spielens: die Schleife wird schneller/langsamer, die Noten bleiben im Takt.
- [ ] D3 OCT− zum Rack: rechts in der Kopfzeile das grüne Dreieck und Takt.Schlag, neben der Oktave; der Soundname
  wird dafür kürzer, aber lesbar. Auf einer Edit-Seite und im Mixer genauso.
- [ ] D4 PLAY im Rack stoppt die Schleife; alle Noten des Loopers verstummen (kein hängender Ton). PLAY startet von
  vorn (Takt 1).
- [ ] D5 K4 CLICK auf OFF, gestoppt, REC: die Aufnahme beginnt sofort, ohne Vorzähler. CLICK ON: es klickt auch beim
  bloßen Abspielen.

## E Stabilität

- [ ] E1 Fünf Minuten: Parts aufnehmen, zurücknehmen, leeren, Tempo und Länge ändern, dabei spielen: keine Aussetzer,
  kein Absturz, kein hängender Ton.
- [ ] E2 Seite neu laden und „Switch on“: die Loops sind weg, TEMPO 120, QUANT 1/16, CLICK REC (Loops liegen nur im
  RAM; das ist so gewollt).
  (Der Browser rechnet anders als das Gerät; Aussetzer hier sagen wenig über das Gerät.)
