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

**Achtung Pfeiltasten:** Hast du zuletzt einen Regler angeklickt, drehen ↑ / ↓ diesen Regler (im Rack K1 = die
Engine!) statt SELECT. Für SELECT den Drehregler SELECT anklicken (das dreht ihn nicht), dann ↑ / ↓.

- [x ] 0.1 `http://localhost:8000` mit Cmd+Shift+R neu laden, „Factory reset“ zweimal klicken (der zweite Klick
  bestätigt), dann „Switch on“. (Falls die Seite nicht lädt, im Terminal:
  `python3 -m http.server 8000 -d firmware/klassik/build/emu`)

## A Tabs

- [x ] A1 SEQ: Looper-Seite, oben die Tabs `LOOP` (hell) und `STEP`. Noch einmal SEQ: Tab STEP, Kopfzeile `STEP 1 VA`;
  sechs Reihen (`1 VA` bis `6 ORGAN`) mit je 16 leeren Steps, unten `NEW STEPS` mit NOTE C4, VEL 100, LEN 1/16
  nebeneinander.
- [ x] A2 OCT+ wechselt ebenso zwischen LOOP und STEP; OCT− schließt die Seite (zurück zum Rack).

## B Steps setzen (Part 1 VA, gestoppt)

- [x ] B1 Q (Step 1): der Step ist gesetzt (orange), oben `BAR 1/1`, die Schleife ist jetzt einen Takt lang.
- [ x] B2 K2 zwei Rasten nach rechts (D4), dann T und A (Steps 5 und 9): gesetzt.
- [x ] B3 T noch einmal (drücken, loslassen): Step 5 ist wieder leer.
- [ x] B4 A festhalten (Shift-Klick): Step 9 weiß umrandet, unten `STEP 09 HELD` mit seinen Werten. K2 nach links:
  seine Note sinkt; K3: sein Anschlag; K4: seine Länge (in der Fußzeile eine Marke bei 100 bzw. 1/16). Loslassen: Step
  9 bleibt gesetzt.
- [x ] B5 HOME halten und K2 drehen (ohne gehaltene Taste): NOTE zurück auf C4, die Marke ist weg.
- [ x] B6 Schwarze Tasten (`2`, `3`, …) tun auf STEP nichts.

## C Abspielen

- [c ] C1 PLAY: die Steps klingen im Takt; die Spalte, die gerade spielt, leuchtet in allen Reihen auf, ihre Nummer oben
  auch; Kopfzeile rechts das grüne Dreieck und Takt.Schlag.
- [x ] C2 Während es spielt, Steps setzen und löschen: Sie klingen ab der nächsten Runde (oder noch in dieser, wenn die
  Abspielposition sie noch nicht erreicht hat).
- [ x] C3 Tab LOOP: die gesetzten Steps erscheinen dort als Noten in Spur 1.
- [x ] C4 Auf LOOP etwas live aufnehmen (REC), dann STEP: die aufgenommenen Noten erscheinen als Steps.
- [ !] C5 Ein ACID-Part (SEL bis Part 3): zwei benachbarte Steps, beim ersten LEN 1/8: ACID gleitet zum zweiten.
- [ x] C6 SEL zu Part 5 (PD) und 6 (ORGAN): dort Steps setzen, sie erscheinen in ihren Reihen und klingen.

Notizen: C5: Länge geht nur für beide benachbarten gleichzeiti einzustellen, oder? und ich denke es wäre sinnvoll,
durch die einzelnen steps navigieren zu können,  dann kann man auch einzelne steps besser auswählen und editieren


## D DRUMS

- [ x] D1 SEL zu Part 2 (DRUMS): Kopfzeile `STEP 2 DRUMS - BD`, Fußzeile `K2 INST BD`; unten die BD-Reihe und die
  Instrumente des Kits als Chips.
- [ x] D2 Steps 1, 5, 9, 13 (Q, T, A, G) für BD; K2 auf SD, Steps 5 und 13 (T, G): Die Tasten zeigen jetzt nur die SD-Steps,
  die Reihe `2 DRUMS` oben alle; bei BD und SD ein Punkt im Chip.
- [ x] D3 PLAY: BD und SD klingen an ihren Steps.
- [ x] D4 HOME + K2: zurück auf BD.

## E Länge und Stabilität

- [x ] E1 Auf LOOP K2 LENGTH 2 Takte, dann STEP und K1 BAR auf 2: leerer Takt 2; dort Steps setzen, sie klingen im
  zweiten Takt.
- [x ] E2 LENGTH zurück auf 1: STEP zeigt Takt 1.
- [x ] E2b Frage: Passt es so (16 Steps je Takt, die Länge = LENGTH des Loopers in Takten)? Oder fehlt dir etwas, z. B.
  8 Steps, eine eigene Länge je Part, ein Raster von 1/8 oder 1/32? (bitte notieren)
- [x ] E3 Fünf Minuten: Steps setzen, löschen, gehalten ändern, zwischen Parts und Tabs wechseln, dabei spielen:
  keine Aussetzer, kein Absturz, kein hängender Ton.
  (Der Browser rechnet anders als das Gerät; Aussetzer hier sagen wenig über das Gerät.)
 
Notizen: Idee : man sollte bars duplizieren können und den inhalt des original bars mitnehmen. also von 16 auf 32 etc
## Auswertung (Claude, 29.10.2026)

- Ohne Befund: Tabs (A1, A2), Steps setzen, löschen, gehalten ändern, HOME-Reset, schwarze Tasten (B1–B6), Abspielen,
  live ändern, LOOP ↔ STEP (C1–C4), Part 5 und 6 (C6), DRUMS mit Instrumenten (D1–D4), Länge und fünf Minuten
  Stabilität (E1–E3). C1 ist mit `c` markiert: als bestanden gewertet (c liegt neben x).
- **E2b (Step-Längen):** „passt so“ → bleibt: 16 Steps je Takt, die Länge = LENGTH des Loopers in Takten
  (DECISIONS.md 29.10.).
- **C5 `[!]` (Länge nur für beide benachbarten Steps zugleich?):** Nein, LEN gilt je Step. Im Simulator nachgestellt
  (Part 3 ACID): nur Step 1 gehalten, K4 auf 1/8, losgelassen, dann Step 2 gesetzt → Step 1 endet auf Tick 92 (1/8),
  Step 2 beginnt auf 48 und bleibt 1/16. Hältst du beide Tasten zugleich (im Emulator: zwei Shift-Klicks), ändert K4
  beide; das ist so gewollt (mehrere Steps auf einmal). Dabei ein **Fehler** gefunden: Nach K2–K4 auf einem
  gehaltenen Step zeigte das Feld unten oft noch den alten Wert (im Simulator auch 330 ms später `LEN 1/16` statt
  1/8), weil die Seite nur neu gezeichnet wurde, wenn sich die Zahl der Noten änderte. Behoben (`lp.stepped` in
  `app.c`); `tests/step.txt` prüft jetzt auch ACID mit Länge je Step (+15 Prüfungen, 783 gesamt). Das hat vermutlich
  so ausgesehen, als ginge die Länge nicht. Außerdem hatten beide Steps dieselbe Note C4: Ein Gleiten auf dieselbe
  Tonhöhe hört man nicht. Der Nachtest gibt Step 2 eine andere Note.
- **Notiz zu C5 (durch die Steps navigieren, um sie einzeln auszuwählen und zu ändern)** und **Notiz zu E (Takte
  duplizieren, mit dem Inhalt, von 16 auf 32 Steps):** zwei neue Funktionen, nach deinem „passt so“ umgesetzt:
  ALGO als Step-Cursor (Mockup 30) und HOME + SEQ verdoppelt die Schleife mit Inhalt (Mockup 31), DECISIONS.md
  29.10.; `tests/step.txt` +51 Prüfungen (834 gesamt). Nachtest N4–N12.

Nachtest (seit 29.10.; Emulator vorher mit Cmd+Shift+R neu laden, Factory reset zweimal, „Switch on“, SEQ zweimal):

- [ ] N1 SEL zu Part 3 (ACID). `Q` festhalten (Shift-Klick), K4 eine Raste nach rechts: unten **sofort** `LEN 1/8`.
  `Q` loslassen (noch einmal klicken). K2 zwei Rasten nach rechts (`NOTE D4`), dann `W` antippen: Step 2 gesetzt.
  PLAY: ACID gleitet von C4 nach D4 (kein neuer Anschlag).
- [ ] N2 PLAY noch einmal (Stopp). `Q` festhalten, K4 eine Raste nach links (`LEN 1/16`), loslassen. PLAY: jetzt zwei getrennte Töne,
  kein Gleiten.
- [ ] N3 Einen Step festhalten und K2, K3, K4 Raste für Raste drehen: NOTE, VEL und LEN unten folgen jeder Raste
  sofort.

Nachtest neue Funktionen (seit 29.10., Mockups 30 `Step-Cursor` und 31 `Loop-Double`; Emulator mit Cmd+Shift+R neu
laden, Factory reset zweimal, „Switch on“, SEQ zweimal; ALGO ist im Emulator der Drehregler **ALGORITHM** links
neben dem Display: Mausrad darüber, oder ihn anklicken und ↑ / ↓. Neuer Stand geladen? HOME + SEQ auf leerer
Schleife zeigt `DOUBLE NO LOOP`):

- [x ] N4 Part 1 (VA): `Q`, `T`, `A` antippen (Steps 1, 5, 9). ALGO eine Raste nach rechts: Step 1 der Reihe `1 VA`
  weiß umrandet, oben die `1` hell und unterstrichen, unten `STEP 01` und rechts `ALGO`, darunter
  `ALGO: A STEP. K2-K4 CHANGE IT. KEYS: SET, CLEAR`.
- [ x] N5 Ohne eine Taste zu halten: K2 zwei Rasten nach rechts (`D4`), K4 eine Raste (`1/8`): nur Step 1 ändert sich
  (beim Abspielen hörbar), die Steps 5 und 9 bleiben C4 und 1/16.
- [ x] N6 ALGO weiter nach rechts: auf Step 2 (leer) steht unten wieder `NEW STEPS`, K2–K4 ändern jetzt die Werte
  neuer Steps; auf Step 5 wieder `STEP 05 ALGO`.
- [ x] N7 HOME (`Esc`) halten und ALGO drehen: kein Rahmen mehr, `NEW STEPS`. ALGO ganz nach links: bleibt so.
- [ x] N8 Mit dem Cursor auf Step 9 die Taste `A` antippen: Step 9 wird gelöscht wie bisher.
- [ x] N9 Tab LOOP, K2 LENGTH auf 2 Takte, zurück zu STEP: ALGO über Step 16 hinaus → `BAR 2/2`, Step 1 von Takt 2
  umrandet; zurück nach links → `BAR 1/2`, Step 16.
- [ x] N10 LENGTH wieder 1 Takt, ein paar Steps auch auf Part 2 (DRUMS). HOME halten und SEQ: Kopfzeile kurz
  `DOUBLE 2 BARS`, LENGTH `2 BARS`, auf LOOP stehen in Takt 2 dieselben Noten wie in Takt 1, bei allen Parts. PLAY:
  zwei gleiche Takte.
- [! ] N11 HOME + SEQ noch dreimal: 4, 8, 16 Takte. Noch einmal: `DOUBLE MAX 16 BARS`, nichts ändert sich.
- [ x] N12 Factory reset, dann gleich HOME + SEQ auf der Looper-Seite: `DOUBLE NO LOOP`. REC (Vorzähler läuft) und
  HOME + SEQ: `DOUBLE AFTER TAKE`.

Notizen: wenn man oben dupliziert hat, kann man nicht mehr runterhalbieren..ist schlecht, wenn man einmal zuviel auf 
douple bar geklickt hat

## Auswertung Nachtest (Claude, 29.10.2026)

- N4–N10 und N12 ohne Befund (`[x ]`, `[ x]` als bestanden gewertet): Step-Cursor auf ALGO, Ändern ohne gehaltene
  Taste, NEW STEPS auf leeren Steps, HOME + ALGO, Taste löscht weiter, Cursor über die Takte, DOUBLE mit allen Parts,
  die Meldungen `NO LOOP` und `AFTER TAKE`. Dass ALGO erst nichts tat, lag am alten Stand im Browser-Cache (nach
  Cmd+Shift+R ging es; im eigenen Chrome-Tab nachgeprüft).
- N1 hast du im Chat bestätigt („glide zweier Noten geht“), N2 und N3 sind nicht markiert: bitte noch nachholen.
- **N11 `[!]` (nach einem DOUBLE zu viel kein Halbieren):** Im Simulator nachgestellt: DOUBLE bis 16 Takte und
  `MAX 16 BARS` gehen; zurück ging es nur über K2 LENGTH, eine Raste je Takt (16 → 8 = acht Rasten). Du hast
  HOME + OCT− als HALF gewählt, umgesetzt (DECISIONS.md 29.10., Mockup 31 ergänzt): 16 → 8 → 4 → 2 → 1, ungerade
  aufgerundet, die Takte danach ausgeblendet wie mit K2 LENGTH, ein DOUBLE danach kopiert wieder darüber.
  `tests/step.txt` +13 Prüfungen (847 gesamt).

Nachtest HALF (Emulator mit Cmd+Shift+R neu laden, Factory reset zweimal, „Switch on“):

- [ x] N2 und N3 von oben.
- [! ] N13 SEQ zweimal (STEP), ein paar Steps setzen, HOME (`Esc`) halten und SEQ dreimal: `DOUBLE 8 BARS`. HOME
  halten und OCT− (`Y`): `HALF 4 BARS`, die Seite bleibt offen, auf LOOP stehen die Noten wie vor dem letzten DOUBLE.
- [ x] N14 HOME + OCT− weiter bis `HALF 1 BAR`, dann `HALF MIN 1 BAR`. Takt 1 klingt wie am Anfang.
- [ x] N15 OCT− allein schließt die Looper-Seite wie bisher.

Notizen: ich denke es macht Sinn, wenn man home+seq gedrückt hat, man generell mit okt + double bar und mit okt - 
half bar machen kann - das ist dann konsitenter

## Auswertung Nachtest HALF (Claude, 29.10.2026)

- N2, N3 (Gleiten nur mit überlappender Länge, Anzeige folgt jeder Raste), N14 (HALF bis 1 Takt, `MIN 1 BAR`) und
  N15 (OCT− schließt die Seite) ohne Befund (`[ x]` als bestanden gewertet). N13 lief (N14 baut darauf auf).
- **N13 `[!]`, Notiz (OCT+ verdoppeln, OCT− halbieren, konsistenter):** Du hast gewählt: HOME halten, OCT+ verdoppelt,
  OCT− halbiert, beliebig oft hintereinander; HOME + SEQ ist wieder nur SEQ (Tab). Umgesetzt (DECISIONS.md 29.10.,
  MANUAL, Mockup 31); in `tests/step.txt` die DOUBLE-Prüfungen auf HOME + OCT+ umgestellt und +6 Prüfungen
  (853 gesamt).

Nachtest HOME + OCT+ (Emulator mit Cmd+Shift+R neu laden, Factory reset zweimal, „Switch on“, SEQ zweimal):

- [ x] N16 HOME (`Esc`) halten und OCT+ (`X`): `DOUBLE NO LOOP`, der Tab bleibt STEP. HOME loslassen, ein paar Steps
  setzen.
- [ x] N17 HOME halten, OCT+ dreimal (`DOUBLE 2 BARS`, `4 BARS`, `8 BARS`), dann OCT− einmal (`HALF 4 BARS`). HOME
  loslassen: LENGTH `4 BARS`, die Noten in allen vier Takten wie in Takt 1.
- [x ] N18 Ohne HOME: OCT+ wechselt LOOP/STEP, OCT− schließt die Seite, wie bisher. HOME + SEQ wechselt nur den Tab.

## Auswertung Nachtest HOME + OCT+ (Claude, 29.10.2026)

- N16–N18 ohne Befund (`[ x]`, `[x ]` als bestanden gewertet): `DOUBLE NO LOOP` ohne Tabwechsel, HOME gehalten
  verdoppelt OCT+ mehrmals und halbiert OCT−, ohne HOME bleiben OCT+ / OCT− und SEQ wie bisher. Deine Häkchen kamen
  schon mit `59d97d2` ins Repo (gespeichert kurz vor dem Commit).
- Damit ist STEP fertig: alle Punkte bestanden, die Notizen C5 (Step-Cursor), E (DOUBLE), N11 (HALF) und N13
  (HOME + OCT+ / OCT−) umgesetzt.
