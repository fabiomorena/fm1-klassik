# LFO – Testprotokoll (Browser-Emulator)

Stand v0.7 Teil 1 (LFO je Part). Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/app.c`,
`firmware/klassik/dsp/engines.c`), dem Host-Test (`firmware/klassik/tests/lfo.txt`) und einer Messung im Simulator
(RESEARCH.md „LFO“). Es geht um Funktion und Stabilität; wie stark und wie schön es klingt, stimmen wir zum Schluss
ab (Notizen dazu sind trotzdem willkommen).

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

**Achtung Pfeiltasten:** Hast du zuletzt einen Regler angeklickt, drehen ↑ / ↓ diesen Regler statt SELECT. Für
SELECT den Drehregler SELECT anklicken (das dreht ihn nicht), dann ↑ / ↓.

Im Emulator: HOME = `Esc`, OCT− = `Y`, OCT+ = `X`, Taste festhalten: Shift-Klick (noch einmal klicken löst sie) oder
die Taste auf der Tastatur gedrückt halten (`Q` … `I`, `A` … `K`). Für Tonhöhe und Lautstärke am besten Kopfhörer.

## 0 Vorbereitung

- [ ] 0.1 `http://localhost:8000` mit **Cmd+Shift+R** neu laden, „Factory reset“ zweimal klicken (der zweite Klick
  bestätigt), dann „Switch on“. Neuer Stand? Der Taster LFO öffnet eine Seite (statt `LFO SOON`).

## A Seite und Tabs

- [x ] A1 LFO: Kopfzeile `LFO 1 VA`, LFO leuchtet; Tabs `LFO` (hell) und `MORE`, rechts `120 BPM`. Oben `SIN - PITCH`
  und `1/8 = 250 MS`, eine flache Welle mit einem weißen Punkt, der wandert; darunter sechs Kacheln, `2 DRUMS` grau
  mit `-`, die anderen `PIT 0`. Fußzeile `K1 RATE 1/8`, `K2 DEPTH 0%`, `K3 WAVE SIN`, `K4 DEST PITCH`.
- [ x] A2 LFO oder OCT+ wechselt zu `MORE` (Fußzeile `K1 SYNC ON`, `K2 RETRIG FREE`, `K3 FADE 0.0 S`, `K4 -`) und
  wieder zurück; OCT− schließt die Seite (zurück zum Rack).
- [ x] A3 Von der LFO-Seite springen EDIT, GLO, SEQ und FX direkt zu ihrer Seite, und von dort LFO hierher. Die Tasten
  spielen überall.

## B Ein Part (Part 1 VA, SAW LEAD)

Eine Taste festhalten (z. B. `Q` gedrückt halten) und dabei drehen.

- [x ] B1 K2 DEPTH auf etwa 10: ein leichtes Vibrato; auf 30: deutlich (etwa ±1 Halbton); auf 100: ein Heulen über
  ±1 Oktave. Die Welle im Bild wird höher, die Kachel `1 VA` zeigt `PIT` und die Tiefe.
- [ x] B2 K1 RATE: langsamer (bis `4 BARS`) und schneller (bis `1/32`); das Vibrato folgt, die Zeile oben auch
  (`1/16 = 125 MS`). DEPTH wieder auf etwa 30.
- [ ] B3 K3 WAVE: `TRI` (gleichmäßig auf und ab), `SAW` (steigt, springt zurück), `SQR` (Triller zwischen zwei Tönen),
  `S&H` (zufällige Stufen). Das Bild zeigt jede Form. Zurück auf `SIN`.
- [ x] B4 K4 DEST `FILTER`: statt der Tonhöhe öffnet und schließt das Filter (Wah); `AMP`: die Lautstärke pulsiert
  (bei DEPTH 100 bis zur Stille). `SHAPE`: bei SAW LEAD hört man nichts (erwartet, siehe unten).
- [x ] B5 HOME (`Esc`) halten und K1–K4 drehen: zurück auf 1/8, 0 %, SIN, PITCH; die Marken in der Fußzeile sind weg.

## C MORE: SYNC, RETRIG, FADE

DEPTH etwa 40, DEST PITCH, dann Tab MORE.

- [ x] C1 K1 SYNC nach links (`OFF`): auf dem Tab LFO steht RATE jetzt in Hz (`5.0 HZ`), K1 geht von `0.10 HZ` bis
  `20 HZ`; das Vibrato hängt nicht mehr am Tempo. Zurück auf `ON`.
- [ x] C2 K2 RETRIG `KEY`: jede neue Note beginnt das Vibrato von vorn (gut bei SQR oder SAW zu hören: jeder Anschlag
  gleich). `FREE`: die Welle läuft durch, egal wann die Note kommt.
- [ x] C3 K3 FADE auf `1.0 S`: nach jedem Anschlag setzt das Vibrato erst langsam ein (verzögertes Vibrato). Das Bild
  zeigt die Welle wachsen. Zurück auf `0.0 S`.
- [x ] C4 Im Takt: SYNC ON, RETRIG FREE, RATE 1/4, DEST AMP, DEPTH 100. Looper (SEQ) mit einer Aufnahme oder ein paar
  Steps, CLICK ON, PLAY: das Pulsieren liegt auf dem Klick, jede Runde gleich. TEMPO ändern: es folgt.

## D Andere Engines

- [x ] D1 SEL zu Part 5 (PD): K4 DEST bietet `PITCH`, `DCW`, `AMP`; `DCW` färbt den Klang (heller, dunkler).
- [ x] D2 Part 6 (ORGAN): nur `PITCH` und `AMP` (Vibrato, Tremolo).
- [x ] D3 Part 4 (FM6): `PITCH` und `AMP`; E.PIANO mit AMP klingt wie ein Tremolo-E-Piano.
- [x ] D4 Part 3 (ACID): `PITCH` und `FILTER`; FILTER bei 1/16 und hoher Tiefe: der typische „wobbelnde“ Bass.
- [ x] D5 Part 2 (DRUMS): oben `NO LFO`, K4 DEST bleibt `-`, auch mit DEPTH ändert sich nichts.
- [ x] D6 VA mit SHAPE hörbar machen: EDIT, K1 WAVE auf `SYNC` (oder `PWM`), zurück, LFO, DEST `SHAPE`: jetzt ändert
  sich der Klang. (Bei SAW, SQR, TRI, SIN und SUB hört man SHAPE nicht: so ist die Engine gebaut.)
- [ x] D7 Part 1 mit DEST FILTER, dann im Rack mit K1 auf ORGAN: Die LFO-Seite zeigt jetzt `PITCH` (ORGAN hat kein
  FILTER, es gilt sein erstes Ziel). Zurück auf VA: wieder `FILTER`.

## E Zusammen

- [ x] E1 Mehrere Parts mit LFO zugleich (VA PITCH, ACID FILTER, ORGAN AMP), dazu der Looper: alles klingt, nichts
  bricht ab, keine Aussetzer.

## F Speichern und Stabilität

- [x ] F1 LFOs auf zwei, drei Parts einstellen (RATE, DEPTH, WAVE, DEST, SYNC, RETRIG, FADE), SAVE, Seite neu laden
  und „Switch on“: alle LFOs wie vorher.
- [ x] F2 Fünf Minuten spielen, dabei auf der LFO-Seite drehen, Parts und Seiten wechseln: keine Aussetzer, kein
  Absturz, kein hängender Ton.

## Auswertung (Claude, 29.10.2026)

- 21 von 22 Punkten ohne Befund (`[x ]`, `[ x]` als bestanden gewertet): die Seite mit beiden Tabs und den Sprüngen zu
  den anderen Seiten, DEPTH und RATE als Vibrato, die Ziele FILTER und AMP, der HOME-Reset, SYNC (Hz), RETRIG, FADE,
  im Takt mit dem Looper (Pulsieren auf dem Klick, folgt dem TEMPO), die Ziele je Engine (PD DCW, ORGAN, FM6
  Tremolo, ACID-Wobble, DRUMS ohne LFO, VA SHAPE mit SYNC/PWM, Ziel bei Engine-Wechsel), mehrere Parts mit Looper,
  Speichern, fünf Minuten Stabilität. 0.1 ist nicht markiert; die anderen Punkte setzen den Start voraus, also gemacht.
- B3 (WAVE TRI, SAW, SQR, S&H) ist nicht markiert, also nicht getestet. Der Host-Test dreht WAVE nur auf TRI und zurück;
  die Formen selbst stehen in `lfo_tick` (`dsp/engines.c`: TRI ab 0 aufwärts, SAW von unten steigend, SQR zwei
  Stufen, S&H ein Zufallswert je Umdrehung). Nach Gehör noch offen.
- Keine Notizen zum Klang: Tiefen und Geschwindigkeiten stimmen wir beim Feinschliff am Ende ab.

### Nachtest

- [ x] B3 Part 1 VA, DEPTH etwa 30, DEST PITCH, eine Taste halten, K3 WAVE durchdrehen: `TRI` gleichmäßig auf und ab,
  `SAW` steigt und springt zurück, `SQR` Triller zwischen zwei Tönen, `S&H` zufällige Stufen; das Bild zeigt jede
  Form.

**Nachtest ausgewertet (Claude, 29.10.2026):** B3 bestanden. Damit ist das LFO-Protokoll fertig.
