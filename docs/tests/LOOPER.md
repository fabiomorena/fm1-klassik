# Looper – Testprotokoll (Browser-Emulator)

Stand v0.6 (sechs Parts). Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/app.c`) und dem
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

**Achtung Pfeiltasten:** Hast du zuletzt einen Regler angeklickt, drehen ↑ / ↓ diesen Regler (im Rack K1 = die
Engine!) statt SELECT. Für SELECT den Drehregler SELECT anklicken (das dreht ihn nicht), dann ↑ / ↓.

- [ x] 0.1 `http://localhost:8000` mit Cmd+Shift+R neu laden, „Factory reset“ zweimal klicken (der zweite Klick
  bestätigt), dann „Switch on“. (Falls die Seite nicht lädt, im Terminal:
  `python3 -m http.server 8000 -d firmware/klassik/build/emu`)

## A Seite und Regler

- [x ] A1 SEQ: Kopfzeile `LOOP EMPTY`, SEQ leuchtet; sechs Spuren untereinander, je Glyphe und Name in einer Zeile:
  `1 VA`, `2 DRUMS`, `3 ACID`, `4 FM6`, `5 PD`, `6 ORGAN`, jede `EMPTY`; Fußzeile `K1 TEMPO 120 BPM`,
  `K2 LENGTH AUTO`, `K3 QUANT 1/16`, `K4 CLICK REC`.
- [ x] A2 PLAY auf der leeren Schleife: Meldung `EMPTY REC FIRST`, nichts startet.
- [x ] A3 K1 drehen: das Tempo ändert sich (40 … 240), im Balken erscheint eine helle Marke bei 120. HOME halten und K1
  drehen: zurück auf 120, die Marke ist weg. Das Gleiche mit K3 (Marke bei 1/16) und K4 (Marke bei REC).
- [ x] A4 OCT− schließt die Seite (zurück zum Rack), SEQ öffnet sie wieder; GLO und EDIT wechseln zu Mixer und
  Edit-Seite.

## B Die erste Aufnahme

- [ x] B1 Part 1 (VA) gewählt, gestoppt, REC: Kopfzeile `LOOP COUNT-IN`, rechts ein roter Punkt und `4`, `3`, `2`, `1`;
  vier Klicks (der erste höher). REC und PLAY leuchten (PLAY rot). Vor der Spur 1 ein roter Punkt.
- [x ] B2 Nach dem Vorzähler: `REC 1 VA`, rechts roter Punkt und Takt.Schlag (`1.1`, `1.2`, …). Zwei Takte lang
  etwas spielen, kurz nach Beginn von Takt 3 REC: `PLAY`, die Länge ist 2 Takte (`K2 LENGTH 2 BARS`), rechts ein
  grünes Dreieck. Die Schleife wiederholt, was du gespielt hast; die Noten erscheinen als Striche in Spur 1.
- [x ] B3 Die Noten sitzen auf dem 1/16-Raster (leicht ungenau gespielt klingt es trotzdem gerade).
- [ x] B4 Beim Aufnehmen klickt es auf jedem Schlag, nach REC nicht mehr (CLICK REC).
- [ x] B5 REC während des Vorzählers bricht ab: gestoppt, nichts aufgenommen, LENGTH bleibt AUTO.

## C Overdub, Zurücknehmen, Leeren

- [ x] C1 Während die Schleife spielt: SEL zu Part 2 (DRUMS), REC (ohne Vorzähler), Drums dazuspielen, REC. Beide
  Spuren spielen zusammen weiter.
- [x ] C2 HOME + REC: Meldung `UNDO 2 DRUMS`, die Drums sind weg; noch einmal HOME + REC: `REDO 2 DRUMS`, wieder da.
- [x ] C3 Auf Part 1 wechseln, HOME + PLAY: Meldung `CLEAR 1 VA? OCT+`. OCT−: `KEPT 1 VA`, nichts geändert (die Oktave
  auch nicht). Noch einmal HOME + PLAY, dann OCT+: `CLEARED 1 VA`, Spur 1 leer, die Drums spielen weiter.
  HOME + REC: Spur 1 wieder da.
- [ x] C4 REC auf Part 1, dann während der Aufnahme SEL: `REC 2 DRUMS`, die Aufnahme ist mit zum nächsten Part
  gewandert.
- [x ] C5 Während die Schleife spielt, eine andere Note festhalten: Sie klingt weiter, bis du loslässt. (Spielt die
  Schleife genau die Note, die du hältst, beendet ihr Ende auch deine; das ist bekannt.)
- [x ] C6 SEL zu Part 5 (PD), REC, etwas dazuspielen, REC; dann Part 6 (ORGAN) genauso. Die Noten erscheinen in ihren
  Spuren; der rote Punkt steht immer vor der Spur, die gerade aufnimmt.
- [ !] C7 Die Spuren sind flach (24 px): Sind die Noten, vor allem die DRUMS, noch gut zu erkennen? (bitte notieren)

Notizen: C3: es löscht sich immer nur die letzte aufgenommen Spur im Looper, auch wenn ich andere auswähle, 
bezieht es sich nur auf die letzte - sollte so ein, dass wenn ich explizit auf eine vorherige Aufnahem gehe und home 
und rec drücke, diese gelöscht wird
C4: ich habe jetzt zweimal VA ud keine drum spur mehr, irgendwie habeich das überschriebn, was nicht vorkomne sollte, 
oder?
wir lassen es erst einmal so mit dem layout, feinschliff später

## D Länge, Takt, Kopfzeile

- [ x] D1 K2 LENGTH auf 4: die Spuren zeigen 4 Takte, die Takte 3 und 4 sind leer; die Marke im Balken steht bei 2.
  HOME + K2: zurück auf 2. Auf 1: nur der erste Takt spielt, Takt 2 ist ausgeblendet; zurück auf 2: wieder da.
- [ x] D2 K1 TEMPO während des Spielens: die Schleife wird schneller/langsamer, die Noten bleiben im Takt.
- [! ] D3 OCT− zum Rack: rechts in der Kopfzeile das grüne Dreieck und Takt.Schlag, neben der Oktave; der Soundname
  wird dafür kürzer, aber lesbar. Auf einer Edit-Seite und im Mixer genauso.
- [x ] D4 PLAY im Rack stoppt die Schleife; alle Noten des Loopers verstummen (kein hängender Ton). PLAY startet von
  vorn (Takt 1).
- [ x] D5 K4 CLICK auf OFF, gestoppt, REC: die Aufnahme beginnt sofort, ohne Vorzähler. CLICK ON: es klickt auch beim
  bloßen Abspielen.

Notizen: D3 (Screenshot entfernt 30.10.2026, in Git bis 642f840) nicht ganz so wie beschrieben


## E Stabilität

- [x ] E0 Auf allen sechs Parts etwas aufnehmen und zusammen spielen lassen: nichts bricht ab, keine Aussetzer. Werden
  es zu viele Stimmen, endet die älteste (wie bisher).
- [ x] E1 Fünf Minuten: Parts aufnehmen, zurücknehmen, leeren, Tempo und Länge ändern, dabei spielen: keine Aussetzer,
  kein Absturz, kein hängender Ton.
- [ x] E2 Seite neu laden und „Switch on“: die Loops sind weg, TEMPO 120, QUANT 1/16, CLICK REC (Loops liegen nur im
  RAM; das ist so gewollt).
  (Der Browser rechnet anders als das Gerät; Aussetzer hier sagen wenig über das Gerät.)

## Auswertung (Claude, 29.10.2026)

Ohne Befund: Seite und Regler, die erste Aufnahme, Vorzähler, Klick, Overdub, Leeren (HOME + PLAY), Aufnahme mit SEL
weiterreichen, Part 5 und 6, Länge, Tempo, Stoppen, CLICK OFF/ON, alle sechs Parts zusammen, fünf Minuten Stabilität.
Nachgemessen im Host-Simulator (Skript im Scratchpad, wie `tests/looper.txt`).

- **Notiz zu C2/C3 (HOME + REC nimmt immer die letzte Aufnahme zurück):** So gebaut: Es gibt *eine* Stufe
  Rückgängig für die letzte Aufnahme, egal welcher Part gewählt ist (`lp_undo`, `lp_swap` in `app.c`). Im Simulator:
  Part 1 aufgenommen, dann Part 2, SEL zurück zu Part 1, HOME + REC: Meldung `UNDO 2 DRUMS`, Part 2 ist leer, Part 1
  unverändert. Deine Erwartung (Rückgängig je gewähltem Part) ist eine Änderung, siehe Frage 1 im Chat. Eine ältere
  Spur ganz löschen geht schon heute: Part wählen, HOME + PLAY, OCT+.
- **C4 (zweimal VA, keine Drum-Spur):** Die Noten sind nicht überschrieben. Dein Screenshot zeigt `2 ORGAN`: Part 2
  hatte eine andere Engine. Das kann nur K1 im Rack (`engine_step`, einzige Stelle). Die Noten bleiben dabei
  liegen und spielen dann mit der neuen Engine (im Simulator: Part 2 von DRUMS auf ORGAN, `events 4` vorher und
  nachher); die Spur heißt dann `2 ORGAN` bzw. `2 VA`. Wahrscheinliche Ursache (unsicher, nicht nachgestellt): die
  Pfeiltasten. Hast du einen Regler angeklickt, drehen ↑ / ↓ **diesen** Regler statt SELECT
  (`base/web/emu/index.html`, Zeilen 375–381 und 449); im Rack ist K1 die Engine, auf der Looper-Seite das Tempo.
  Auf dem Gerät gibt es das nicht. In den Tabellen der Protokolle steht jetzt ein Hinweis.
- **D3 (keine Anzeige im Rack):** Im Simulator zeigt das Rack mit `2 ORGAN` bei laufender Schleife rechts `▶ 2.3`,
  der Name wird zu `FULL ORGA` gekürzt. Auf deinem Screenshot steht `FULL ORGAN` ganz und kein Dreieck: so sieht das
  Rack aus, wenn die Schleife **steht** (die Anzeige gibt es nur beim Spielen und Aufnehmen, `loop_ind`). Lief sie
  in dem Moment? Siehe Nachtest N3. Dass der Name dafür abgeschnitten wird (`FULL ORGA`), ist Feinschliff.
- **C7 (Spurhöhe):** Laut Notiz bleibt das Layout so; Feinschliff am Ende (`docs/DECISIONS.md`).

Nachtest:

- [ !] N1 Im Rack K1 anklicken, dann ↓ drücken: Die Engine des gewählten Parts wechselt (bestätigt die Ursache von C4).
  Danach den Drehregler SELECT anklicken: ↑ / ↓ wählen wieder den Part.
- [ x] N2 Part 2 (DRUMS) aufnehmen, im Rack K1 auf ORGAN und zurück auf DRUMS: Die Drum-Spur ist wieder da und spielt
  Drums.
- [ x] N3 Schleife mit PLAY starten (grünes Dreieck auf der Looper-Seite), dann OCT−: Im Rack steht rechts das grüne
  Dreieck mit Takt.Schlag. Falls nicht: Screenshot und notieren, auf welcher Seite du PLAY gedrückt hast.

Nachtest Rückgängig je Part (neu seit 29.10., deine Entscheidung zu Frage 1; Emulator vorher mit Cmd+Shift+R neu
laden):

- [ x] N4 Part 1 (VA) aufnehmen, dann Part 2 (DRUMS) dazu. SEL zurück zu Part 1, HOME + REC: Meldung `UNDO 1 VA`, Spur
  1 ist leer, die Drums spielen weiter, die Länge bleibt. Noch einmal HOME + REC: `REDO 1 VA`, Spur 1 wieder da.
- [x ] N5 SEL zu Part 2, HOME + REC: `UNDO 2 DRUMS`, nur die Drums sind weg. Auf einem Part ohne Aufnahme (z. B. 3):
  `UNDO NOTHING`.
- [ x] N6 Nur ein Part mit Aufnahme (die erste), HOME + REC: Spur leer, LENGTH wieder AUTO, die Schleife steht (wie
  bisher).


Notizen: N1: (Screenshot entfernt 30.10.2026, in Git bis 642f840) warum hat FM& jetzt 1/4/6 zugewiesen bekommen, wie das geht habe ich noch nicht 
verstanden


## Auswertung Nachtest (Claude, 29.10.2026)

- N2–N6 bestanden: Die Noten bleiben beim Engine-Wechsel liegen, das Rack zeigt die laufende Schleife (D3 war also
  eine stehende Schleife), Rückgängig je Part geht wie geplant.
- **N1 (`[!]`, warum spielen 1, 4 und 6 FM6):** Das ist die Ursache von C4, jetzt bestätigt. Mit angeklicktem K1 wechselt
  jedes ↓ die Engine des **gewählten** Parts um eine Kachel nach links, in der Reihenfolge des Racks: FM6, VA, PD,
  ORGAN, DRUMS, ACID, und von FM6 weiter zu ACID (`engine_step` in `app.c`). Part 4 spielt FM6 von Anfang an; Part 1
  (VA) ist eine Kachel nach links zu FM6 geworden, Part 6 (ORGAN) drei (ORGAN → PD → VA → FM6). Wie oft du wo ↓
  gedrückt hast, weiß ich nicht; das Ergebnis passt dazu.
  Die Kreise auf einer Kachel sind die Parts, die diese Engine spielen (gefüllt: der gewählte). Mehrere Parts dürfen
  dieselbe Engine haben, z. B. zwei FM6-Parts mit verschiedenen Sounds (PARTS B2). Zurück: den Part wählen und K1
  drehen, bis seine Engine wieder stimmt, oder „Factory reset“. Auf dem Gerät passiert das nur, wenn du an K1 drehst.
