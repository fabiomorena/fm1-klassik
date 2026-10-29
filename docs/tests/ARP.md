# ARP – Testprotokoll (Browser-Emulator)

Stand v0.7 Teil 2 (Arpeggiator je Part). Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/app.c`)
und dem Host-Test (`firmware/klassik/tests/arp.txt`). Es geht um Funktion und Stabilität; wie es klingt und aussieht,
stimmen wir zum Schluss ab (Notizen dazu sind trotzdem willkommen).

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

Im Browser nicht prüfbar: MIDI-In in den Arp und die Arp-Noten auf USB-MIDI-Out (der Emulator hat kein Web-MIDI; das
prüft der Host-Test), die Rechenlast auf dem Gerät.

## 0 Vorbereitung

| Was | Wie im Emulator |
| --- | --- |
| Regler drehen | Mausrad über dem Regler, oder Regler anklicken und Pfeiltasten (Bild↑ / Bild↓: 5 Schritte) |
| Taste oder Taster festhalten | Rechtsklick oder Shift-Klick: bleibt gedrückt, bis du noch einmal klickst |
| weiße Tasten | `Q W E R T Z U I` = F3 … F4, `A S D F G H J K` = G4 … G5. Ein Akkord C-Dur: `T` (C4), `U` (E4), `A` (G4) zusammen gedrückt halten |
| PLAY, REC, HOME | `P`, `O`, `Esc` |
| OCT− / OCT+ | `Y` / `X` |
| SEL, SELECT | SEL anklicken; SELECT: Pfeiltasten ↑ / ↓ |

Die Tastatur-Kürzel wirken nur, wenn das Panel den Fokus hat: einmal aufs Gerät klicken. Am bequemsten geht der Arp
mit **LATCH** (Teil C): Akkord einmal anschlagen, er spielt weiter, die Hände sind frei für die Regler.

**Achtung Pfeiltasten:** Hast du zuletzt einen Regler angeklickt, drehen ↑ / ↓ diesen Regler statt SELECT. Für
SELECT den Drehregler SELECT anklicken (das dreht ihn nicht), dann ↑ / ↓.

- [ x] 0.1 `http://localhost:8000` mit **Cmd+Shift+R** neu laden, „Factory reset“ zweimal klicken (der zweite Klick
  bestätigt), dann „Switch on“. Neuer Stand? Der Taster ARP öffnet eine Seite (statt `ARP SOON`). (Falls die Seite
  nicht lädt, im Terminal: `python3 -m http.server 8000 -d firmware/klassik/build/emu`)

## A Seite

- [ x] A1 ARP: Kopfzeile `ARP 1 VA`, ARP leuchtet; Tab `ARP`, rechts `120 BPM`. Oben `ARP OFF` und ein Kästchen
  `LATCH` (dunkel), darunter `1 VA: KEYS PLAY AS ALWAYS`; sechs Kacheln, alle `OFF`. Fußzeile `K1 MODE OFF`,
  `K2 RATE 1/16`, `K3 OCT 1`, `K4 GATE 50%`.
- [ x] A2 MODE OFF: die Tasten spielen wie immer (ein Ton, solange du hältst).
- [ ] A3 OCT− schließt die Seite (zurück zum Rack). Von der ARP-Seite springen EDIT, GLO, SEQ, FX und LFO direkt zu
  ihrer Seite, und von dort ARP hierher.

## B Ein Part (Part 1 VA, SAW LEAD)

- [ ] B1 K1 MODE `UP`. C-Dur halten (`T`, `U`, `A`): C4, E4, G4, C4 … in 1/16 (8 Töne je Sekunde), jeder halb so
  lang wie der Schritt. Oben `UP - 1/16 - 1 OCT`; im Bild 16 Balken in der Höhe ihrer Note, eine weiße Linie wandert,
  links von ihr hell (gespielt), rechts dunkel (was kommt); darunter `HELD C4 E4 G4 - GATE 50%`. Kachel `1 VA`:
  `UP` und `1/16`. Loslassen: der Arp hört auf, unten `HOLD KEYS`.
- [x ] B2 `DOWN`: G4, E4, C4, G4 … (beginnt oben). `UP-DN`: C4, E4, G4, E4, C4, E4 … (oben und unten nicht doppelt).
- [x ] B3 `RAND`: die drei Noten durcheinander, nie zweimal dieselbe hintereinander; das Bild zeigt rechts der Linie,
  was eine Runde vorher kam.
- [ x] B4 `ORDER`: erst `A` (G4), dann `T` (C4), dann `U` (E4) drücken und halten: G4, C4, E4, G4 … (wie gedrückt, nicht
  von tief nach hoch); `HELD G4 C4 E4`.
- [x ] B5 K3 OCT `2`, `3`, `4` (MODE UP): nach C4 E4 G4 dasselbe eine, zwei, drei Oktaven höher, dann wieder von unten.
  Im Bild steigen die Balken weiter. Zurück auf `1`.
- [x ] B6 K2 RATE von `1/4` (langsam) bis `1/32` (schnell), dazwischen punktiert (`1/8.`, `1/16.`) und triolisch
  (`1/4T`, `1/8T`, `1/16T`). TEMPO auf der Looper-Seite (SEQ, K1) ändern, zurück zu ARP: der Arp folgt.
- [x ] B7 K4 GATE: `10%` ganz kurz (Staccato), `100%` ohne Lücke; weiter nach rechts geht es auf VA nicht (bleibt
  `100%`). Die Balken im Bild werden kürzer und länger.
- [ x] B8 HOME (`Esc`) halten und K1–K4 drehen: zurück auf `OFF`, `1/16`, `1`, `50%`; die Marken in der Fußzeile sind
  weg.

## C LATCH

MODE `UP`.

- [ ] C1 ARP (auf der Seite): `LATCH` wird hell (`LATCH ON`). C-Dur kurz anschlagen und loslassen: der Arp spielt
  weiter.
- [ ] C2 Einen neuen Akkord anschlagen (z. B. `R` B3, `U` E4): er ersetzt den alten. Während du ihn hältst, kommt eine
  weitere Taste dazu, statt neu anzufangen.
- [ ] C3 ARP noch einmal: `LATCH` dunkel (`LATCH OFF`), der Arp hört auf (keine Taste gehalten).
- [ ] C4 Zwei Parts zugleich (Schleife gestoppt): Part 1 mit LATCH spielen lassen, SEL zu Part 5 (PD), dort MODE
  `UP`, LATCH, einen anderen Akkord: beide Arps laufen nebeneinander, im selben Takt (der zweite setzt auf dem
  nächsten Schritt des ersten ein, nicht versetzt). Das LATCH-Kästchen zeigt den gewählten Part. Beide wieder aus
  (LATCH aus oder MODE `OFF`).

## D ACID: SLIDE und Accent

- [ ] D1 SEL zu Part 3 (ACID), MODE `UP`, LATCH, zwei Töne (`T`, `A`). K4 GATE über `100%` hinaus: `SLIDE`. Jetzt
  gleiten die Töne ineinander (Portamento), statt neu anzusetzen; die Balken im Bild berühren sich. Zurück auf `50%`:
  wieder einzelne Töne.
- [ ] D2 ENV festhalten und dabei den Akkord neu anschlagen (Kopfzeile zeigt `ACC`): die Arp-Noten klingen mit Accent
  (lauter, heller), auch nachdem ENV losgelassen ist. Ohne ENV neu angeschlagen: wieder ohne.
- [ ] D3 Part 3 auf eine andere Engine stellen (Rack, K1), Arp-Seite: GATE zeigt `100%` statt `SLIDE`. Zurück auf
  ACID: wieder `SLIDE`. Danach LATCH aus.

## E Mit dem Looper

- [ x] E1 Part 1 VA, MODE `UP`, LATCH, C-Dur anschlagen. REC (`O`): ein Takt Vorzählen, dann nimmt der Looper die
  Arp-Noten auf; nach einem Takt REC noch einmal. Die Schleife läuft (1 Takt). HOME + K1 (MODE `OFF`) und LATCH aus:
  die Schleife spielt die Arp-Noten allein weiter, genauso wie aufgenommen. Auf der Looper-Seite (SEQ) die Noten in
  Spur 1 gleichmäßig, wie eine Treppe.
- [x ] E2 Bei laufender Schleife auf Part 5 (PD) MODE `UP` und einen Akkord halten: der Arp setzt im Takt der Schleife
  ein (mit CLICK `ON` auf der Looper-Seite gut zu hören) und bleibt darauf.
- [x ] E3 Triolen: Part 5 RATE `1/16T`, REC über die laufende Schleife, nach einem Takt REC. Auf der Looper-Seite
  liegen die Noten von Spur 5 triolisch (sechs je Schlag), nicht auf die 1/16 von QUANT verschoben.
- [x ] E4 HOME + REC nimmt die Arp-Aufnahme von Part 5 zurück (wie bei einer gespielten).

## F DRUMS

- [ x] F1 Part 2 (DRUMS), MODE `UP`, LATCH, BD `T`, SD `Z` und CH `1` zusammen anschlagen: der Arp spielt sie
  nacheinander, von tief nach hoch: CH (`1` ist F#3), BD (C4), SD (D4), wie ein kleiner Beat. OCT ändert bei DRUMS nichts am Klang (die Oktave wählt kein
  anderes Instrument). Danach LATCH aus, MODE `OFF`.

## G Speichern und Stabilität

- [ x] G1 ARP auf zwei, drei Parts einstellen (MODE, RATE, OCT, GATE), LATCH an, SAVE, Seite neu laden und „Switch on“:
  MODE, RATE, OCT, GATE wie vorher, LATCH aus.
- [ x] G2 Fünf Minuten spielen, dabei auf der ARP-Seite drehen, Parts und Seiten wechseln, mit Schleife und ohne:
  keine Aussetzer, kein Absturz, kein hängender Ton.

## Auswertung (Claude, 29.10.2026)

- 17 Punkte ohne Befund (`[x ]`, `[ x]` als bestanden gewertet): die Seite (A1), OFF (A2), DOWN, UP-DN, RAND, ORDER,
  OCT, RATE mit TEMPO, GATE, HOME-Reset (B2–B8), mit dem Looper: Aufnahme, Abspielen ohne Arp, Einsetzen im Takt der
  Schleife, Triolen ohne QUANT, HOME + REC (E1–E4), DRUMS (F1), Speichern ohne LATCH (G1), fünf Minuten Stabilität
  (G2). Kein `[!]`, keine Notiz.
- Nicht markiert, also nicht getestet: A3, B1, C1–C4, D1–D3. Teilweise abgedeckt: UP spielt in E1, F1 und G1 mit,
  LATCH an und aus in E1 und F1 (C1, C3). Ganz offen: das Ersetzen eines gelatchten Akkords (C2), zwei Arps auf der
  gemeinsamen Uhr (C4), ACID mit SLIDE und Accent (D1–D3), die Sprünge zu den anderen Seiten (A3). Im Host-Test
  (`tests/arp.txt`) sind C2 (das Ersetzen, nicht das Dazukommen), C4 und D1 geprüft (SLIDE: Note-on vor dem Note-off der vorigen am selben Tick); wie
  SLIDE klingt, nur nach Gehör.
- Klang (Tempo, GATE, Pegel der Arp-Noten) stimmen wir beim Feinschliff am Ende ab.

### Nachtest

- [ ] A3 OCT− schließt die Seite (zurück zum Rack). Von der ARP-Seite springen EDIT, GLO, SEQ, FX und LFO direkt zu
  ihrer Seite, und von dort ARP hierher.
- [ ] B1 Part 1 VA, MODE `UP`, C-Dur halten (`T`, `U`, `A`): C4, E4, G4, C4 … in 1/16; im Bild 16 Balken, die weiße
  Linie wandert, links hell, rechts dunkel; darunter `HELD C4 E4 G4 - GATE 50%`; Kachel `1 VA`: `UP` und `1/16`.
  Loslassen: der Arp hört auf, unten `HOLD KEYS`.
- [ ] C2 LATCH an, C-Dur anschlagen, loslassen; dann einen neuen Akkord (`R` B3, `U` E4): er ersetzt den alten. Während
  du ihn hältst, kommt eine weitere Taste dazu, statt neu anzufangen.
- [ ] C4 Schleife gestoppt: Part 1 mit LATCH spielen lassen, SEL zu Part 5 (PD), MODE `UP`, LATCH, ein anderer
  Akkord: beide Arps im selben Takt (der zweite setzt auf dem nächsten Schritt des ersten ein). Beide wieder aus.
- [ ] D1 Part 3 (ACID), MODE `UP`, LATCH, zwei Töne (`T`, `A`). K4 GATE über `100%` hinaus: `SLIDE`, die Töne
  gleiten ineinander; zurück auf `50%`: einzelne Töne.
- [ ] D2 ENV festhalten und den Akkord neu anschlagen (`ACC` in der Kopfzeile): die Arp-Noten mit Accent, auch nach
  dem Loslassen von ENV; ohne ENV neu angeschlagen: wieder ohne.
- [ ] D3 Part 3 im Rack (K1) auf eine andere Engine: GATE zeigt `100%` statt `SLIDE`; zurück auf ACID: wieder
  `SLIDE`. Danach LATCH aus.
