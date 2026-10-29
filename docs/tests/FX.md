# Effekte – Testprotokoll (Browser-Emulator)

Stand v0.6 (sechs Parts). Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/app.c`,
`firmware/klassik/dsp/fx.c`) und dem Host-Test (`firmware/klassik/tests/fx.txt`). Es geht um Funktion und Stabilität;
wie Delay und Hall klingen, stimmen wir zum Schluss ab (Notizen dazu sind trotzdem willkommen).

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

**Achtung Pfeiltasten:** Hast du zuletzt einen Regler angeklickt, drehen ↑ / ↓ diesen Regler (im Rack K1 = die
Engine!) statt SELECT. Für SELECT den Drehregler SELECT anklicken (das dreht ihn nicht), dann ↑ / ↓.

Für links/rechts Kopfhörer nehmen (ob der Lautsprecher des FM-1 stereo ist, ist unbekannt). HOME im Emulator: `Esc`;
Taster festhalten: Shift-Klick.

## 0 Vorbereitung

- [ ] 0.1 `http://localhost:8000` mit Cmd+Shift+R neu laden, „Factory reset“ zweimal klicken (der zweite Klick
  bestätigt), dann „Switch on“. Alle Sends stehen auf 0.

## A Seiten und Tabs

- [ ] A1 FX: Kopfzeile `FX DELAY + HALL`, FX leuchtet; Tabs `DELAY` (hell) und `HALL`, rechts das Tempo
  (`120 BPM`). Oben ein Bild mit `PING-PONG` und `1/8. = 375 MS`, darunter sechs Kacheln (drei nebeneinander, zwei
  Reihen), eine je Part, alle mit `0`.
  Fußzeile `K1 TIME 1/8.`, `K2 FDBK 35%`, `K3 TONE 70%`, `K4 LEVEL 0 dB`.
- [ ] A2 FX oder OCT+ wechselt zu `HALL` (Bild `STEREO`, `PRE 20 MS`; Fußzeile `K1 PRE 20 MS`, `K2 DECAY 50%`,
  `K3 TONE 60%`, `K4 LEVEL 0 dB`) und wieder zurück; OCT− schließt die Seite (zurück zum Rack).
- [ ] A3 GLO: der Mixer auf dem Tab `MIX` (Tab-Zeile `MIX SEND`; auf MIX keine Sends, seit 29.10.). GLO oder OCT+
  wechselt zu `SEND` und zurück; OCT− schließt ihn.
- [ ] A4 Von jeder dieser Seiten springen EDIT, GLO, SEQ und FX direkt zu ihrer Seite. Die Tasten spielen überall.

## B Sends im Mixer (Tab SEND)

- [ ] B1 Auf `SEND`: in jedem Zug zwei senkrechte Fader `DLY` und `HALL` mit ihrem Wert darunter, beim umrandeten Part
  hell. Fußzeile `K1 DELAY 0`, `K2 HALL 0`, `K3 -`, `K4 -`. K1 und K2 drehen: Wert und Fader folgen. K3 und K4 tun
  nichts.
- [ ] B2 SEL: der Rahmen wandert, die Fußzeile zeigt die Sends des nächsten Parts; ab Part 4 die Seite `PARTS 4-6`.
  Die Sends von Part 5 und 6 stehen danach auch auf ihren Kacheln der FX-Seite.
- [ ] B3 Ein Send nicht auf 0: eine helle Marke am Anfang des Balkens in der Fußzeile. HOME (`Esc`) halten und K1/K2
  drehen: zurück auf 0, die Marke ist weg.
- [ ] B4 Im Zug `MASTER` auf `SEND`: zwei graue Balken `DLY` und `HALL`, wie laut die Effekte zurückkommen (LEVEL
  der FX-Seite, darunter der Wert, z. B. `0`).

## C Delay

Part 1 (VA, SAW LEAD), im Mixer auf `SEND` K1 DELAY auf 100.

- [ ] C1 Kurze Noten spielen: Die Echos kommen im Abstand von 1/8. (bei 120 BPM 375 ms), das erste links, das nächste
  rechts, dann wieder links (Kopfhörer), jedes leiser und etwas dunkler. Das erste ist etwa so laut wie die Note.
- [ ] C2 Auf der FX-Seite K1 TIME drehen (1/32 … 1/4): der Abstand ändert sich hörbar, das Bild und die Zeile darüber
  (`1/4 = 500 MS`) folgen. Beim Drehen während der Echos knackt es nicht (Überblenden in 23 ms).
- [ ] C3 K2 FDBK: höher = mehr Echos (bei 100 % sehr lange, aber sie verklingen); 0 % = ein einziges Echo.
- [ ] C4 K3 TONE: niedriger = die Echos werden schneller dunkel.
- [ ] C5 K4 LEVEL: leiser / lauter, ganz links `OFF`: keine Echos mehr.
- [ ] C6 Im Takt: Looper-Seite (SEQ), K4 CLICK auf `ON`, PLAY. Die Echos von 1/8. oder 1/4 liegen auf dem Klick.
  TEMPO ändern: der Abstand der Echos folgt (zurück auf der FX-Seite steht das Tempo rechts in der Tab-Zeile).
- [ ] C7 Langsames Tempo: TEMPO 40 BPM, TIME 1/4: über der Grafik steht `1/16 = 375 MS` (die Viertel wäre 1,5 s, das
  Delay hält 0,5 s, also zweimal halbiert). Die Echos liegen weiter im Takt. Danach TEMPO zurück (HOME + K1).
- [ ] C8 Part 1 im Mixer stumm (MUTE): keine neuen Echos, die alten klingen aus. Die Kachel von Part 1 auf der
  FX-Seite ist grau mit `M`.

## D Hall

Im Mixer auf `SEND` bei Part 1 K1 DELAY auf 0, K2 HALL auf 100.

- [ ] D1 Noten spielen: ein Nachhall, links und rechts etwas verschieden, er klingt nach dem Loslassen nach (bei
  DECAY 50 % etwa 2 Sekunden).
- [ ] D2 Auf der FX-Seite Tab `HALL`, K2 DECAY: höher = länger; das Bild (der Schweif) folgt.
- [ ] D3 K1 PRE auf 100 ms: zwischen Note und Hall eine kleine Lücke (bei kurzen, harten Klängen wie DRUMS am besten zu
  hören); 0 ms: keine Lücke.
- [ ] D4 K3 TONE: dunkler / heller. K4 LEVEL: leiser / lauter, ganz links `OFF`.
- [ ] D5 HOME (`Esc`) halten und K1–K4 drehen: zurück auf 20 MS, 50 %, 60 %, 0 dB; die Marken sind weg.

## E Zusammen

- [ ] E1 DRUMS (Part 2) mit etwas HALL, ACID (Part 3) mit DELAY 1/8. und FDBK um 50 %, dazu der Looper mit einer
  Aufnahme: alles klingt zusammen, nichts bricht ab, keine Aussetzer.
- [ ] E2 Laut: alle Sends auf 100, MASTER voll: `LIM` leuchtet, es verzerrt nicht hart (unsicher: wie es klingt,
  bitte notieren).
- [ ] E3 Part 5 (PD) mit DELAY, Part 6 (ORGAN) mit HALL: beide Effekte klingen.

## F Speichern und Stabilität

- [ ] F1 Sends und Effekte einstellen, SAVE, Seite neu laden und „Switch on“: alle Sends, TIME, FDBK, TONE, LEVEL,
  PRE, DECAY wie vorher.
- [ ] F2 Fünf Minuten spielen, dabei auf der FX-Seite und im Mixer drehen, Parts wechseln: keine Aussetzer, kein
  Absturz.
