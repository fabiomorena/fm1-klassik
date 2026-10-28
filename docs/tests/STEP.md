# Step-Sequencer – Testprotokoll (Browser-Emulator)

Stand v0.6 (sechs Parts). Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/app.c`) und dem
Host-Test (`firmware/klassik/tests/step.txt`). Es geht um Funktion und Stabilität; Klang und Design kommen zum
Schluss.

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

Im Browser nicht prüfbar: die Lichter der Tasten (der Emulator zeigt sie vielleicht nicht; bitte notieren, was du
siehst), Anschlagdynamik.

## 0 Vorbereitung

| Was | Wie im Emulator |
| --- | --- |
| Regler drehen | Mausrad über dem Regler, oder Regler anklicken und Pfeiltasten (Bild↑ / Bild↓: 5 Schritte) |
| Taste oder Taster festhalten | Rechtsklick oder Shift-Klick: bleibt gedrückt, bis du noch einmal klickst |
| weiße Tasten = Steps | `Q W E R T Z U I` = Step 1 … 8, `A S D F G H J K` = Step 9 … 16 |
| PLAY, REC, HOME | `P`, `O`, `Esc` |
| OCT− / OCT+ | `Y` / `X` |

Die Tastatur-Kürzel wirken nur, wenn das Panel den Fokus hat: einmal aufs Gerät klicken.

- [ ] 0.1 `http://localhost:8000` mit Cmd+Shift+R neu laden, „Factory reset“ zweimal klicken (der zweite Klick
  bestätigt), dann „Switch on“. (Falls die Seite nicht lädt, im Terminal:
  `python3 -m http.server 8000 -d firmware/klassik/build/emu`)

## A Tabs

- [ ] A1 SEQ: Looper-Seite, oben die Tabs `LOOP` (hell) und `STEP`. Noch einmal SEQ: Tab STEP, Kopfzeile `STEP 1 VA`;
  sechs Reihen (`1 VA` bis `6 ORGAN`) mit je 16 leeren Steps, unten `NEW STEPS` mit NOTE C4, VEL 100, LEN 1/16
  nebeneinander.
- [ ] A2 OCT+ wechselt ebenso zwischen LOOP und STEP; OCT− schließt die Seite (zurück zum Rack).

## B Steps setzen (Part 1 VA, gestoppt)

- [ ] B1 Q (Step 1): der Step ist gesetzt (orange), oben `BAR 1/1`, die Schleife ist jetzt einen Takt lang.
- [ ] B2 K2 zwei Rasten nach rechts (D4), dann T und A (Steps 5 und 9): gesetzt.
- [ ] B3 T noch einmal (drücken, loslassen): Step 5 ist wieder leer.
- [ ] B4 A festhalten (Shift-Klick): Step 9 weiß umrandet, unten `STEP 09 HELD` mit seinen Werten. K2 nach links:
  seine Note sinkt; K3: sein Anschlag; K4: seine Länge (in der Fußzeile eine Marke bei 100 bzw. 1/16). Loslassen: Step
  9 bleibt gesetzt.
- [ ] B5 HOME halten und K2 drehen (ohne gehaltene Taste): NOTE zurück auf C4, die Marke ist weg.
- [ ] B6 Schwarze Tasten (`2`, `3`, …) tun auf STEP nichts.

## C Abspielen

- [ ] C1 PLAY: die Steps klingen im Takt; die Spalte, die gerade spielt, leuchtet in allen Reihen auf, ihre Nummer oben
  auch; Kopfzeile rechts das grüne Dreieck und Takt.Schlag.
- [ ] C2 Während es spielt, Steps setzen und löschen: Sie klingen ab der nächsten Runde (oder noch in dieser, wenn die
  Abspielposition sie noch nicht erreicht hat).
- [ ] C3 Tab LOOP: die gesetzten Steps erscheinen dort als Noten in Spur 1.
- [ ] C4 Auf LOOP etwas live aufnehmen (REC), dann STEP: die aufgenommenen Noten erscheinen als Steps.
- [ ] C5 Ein ACID-Part (SEL bis Part 3): zwei benachbarte Steps, beim ersten LEN 1/8: ACID gleitet zum zweiten.
- [ ] C6 SEL zu Part 5 (PD) und 6 (ORGAN): dort Steps setzen, sie erscheinen in ihren Reihen und klingen.

## D DRUMS

- [ ] D1 SEL zu Part 2 (DRUMS): Kopfzeile `STEP 2 DRUMS - BD`, Fußzeile `K2 INST BD`; unten die BD-Reihe und die
  Instrumente des Kits als Chips.
- [ ] D2 Steps 1, 5, 9, 13 (Q, T, A, G) für BD; K2 auf SD, Steps 5 und 13 (T, G): Die Tasten zeigen jetzt nur die SD-Steps,
  die Reihe `2 DRUMS` oben alle; bei BD und SD ein Punkt im Chip.
- [ ] D3 PLAY: BD und SD klingen an ihren Steps.
- [ ] D4 HOME + K2: zurück auf BD.

## E Länge und Stabilität

- [ ] E1 Auf LOOP K2 LENGTH 2 Takte, dann STEP und K1 BAR auf 2: leerer Takt 2; dort Steps setzen, sie klingen im
  zweiten Takt.
- [ ] E2 LENGTH zurück auf 1: STEP zeigt Takt 1.
- [ ] E2b Frage: Passt es so (16 Steps je Takt, die Länge = LENGTH des Loopers in Takten)? Oder fehlt dir etwas, z. B.
  8 Steps, eine eigene Länge je Part, ein Raster von 1/8 oder 1/32? (bitte notieren)
- [ ] E3 Fünf Minuten: Steps setzen, löschen, gehalten ändern, zwischen Parts und Tabs wechseln, dabei spielen:
  keine Aussetzer, kein Absturz, kein hängender Ton.
  (Der Browser rechnet anders als das Gerät; Aussetzer hier sagen wenig über das Gerät.)
