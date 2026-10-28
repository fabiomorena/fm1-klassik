# Mixer – Testprotokoll (Browser-Emulator)

Stand v0.6 (sechs Parts). Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/app.c`) und den
Host-Tests (`firmware/klassik/tests/mixer.txt`, `parts.txt`). Es geht um Funktion und Stabilität; Klang und Design
kommen zum Schluss. Die Sends (Tab `SEND`) prüft `FX.md`.

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

**Achtung Pfeiltasten:** Hast du zuletzt einen Regler angeklickt, drehen ↑ / ↓ diesen Regler (im Rack K1 = die
Engine!) statt SELECT. Für SELECT vorher einen Taster anklicken, oder das Mausrad über SELECT nehmen.

Für Pan Kopfhörer nehmen (ob der Lautsprecher des FM-1 stereo ist, ist unbekannt).

## 0 Vorbereitung

- [ ] 0.1 `http://localhost:8000` mit Cmd+Shift+R neu laden, „Factory reset“ zweimal klicken (der zweite Klick
  bestätigt), dann „Switch on“. HOME im Emulator: `Esc`; Taster festhalten: Shift-Klick.

## A Öffnen, Seiten, schließen

- [ ] A1 GLO: Kopfzeile `MIX PARTS 1-3`, GLO leuchtet; darunter die Tabs `MIX` (hell) und `SEND`, rechts zwei kleine
  Marken, die linke hell (Seite 1). Vier Züge: `1 VA`, `2 DRUMS`, `3 ACID`, `MASTER`, die Namen ganz lesbar. Der Zug
  von Part 1 ist umrandet. Fußzeile `K1 LEVEL 0 dB`, `K2 PAN C`, `K3 MUTE OFF`, `K4 SOLO OFF`.
- [ ] A2 SEL: Der Rahmen wandert zu Part 2, die Fußzeile zeigt seine Werte. Die Tasten spielen den umrandeten Part.
- [ ] A2b SEL bis Part 4: die Seite wechselt zu `PARTS 4-6` mit `4 FM6`, `5 PD`, `6 ORGAN`, die rechte Marke ist
  hell. SEL von 6 auf 1: zurück zur ersten Seite. Mit SELECT (Pfeiltasten ↑ / ↓) genauso, auch rückwärts.
- [ ] A3 GLO oder OCT+ wechselt zwischen `MIX` und `SEND`; OCT− schließt den Mixer (zurück zum Rack); EDIT im Mixer
  öffnet stattdessen die Edit-Seite.

## B Ein Part

- [ ] B1 K1 LEVEL: leiser / lauter (bis +6 dB), der Fader folgt; ganz unten `OFF`, der Part ist still.
- [ ] B2 K2 PAN (Kopfhörer): nach links nur noch links, nach rechts nur rechts, Mitte beide.
- [ ] B3 K3 MUTE nach rechts: `M` leuchtet, der Part ist still und grau; nach links wieder da.
- [ ] B4 K4 SOLO auf einem Part: alle anderen Parts verstummen (grau), auch die auf der anderen Seite. Solo auf einem
  zweiten Part: beide klingen.
- [ ] B5 LEVEL nicht auf 0 dB oder PAN nicht in der Mitte: Eine helle Marke im Balken der Fußzeile zeigt 0 dB bzw.
  die Mitte. HOME (`Esc`) halten und K1–K4 drehen: zurück auf 0 dB, Mitte, aus; die Marken sind weg.
- [ ] B6 Die Änderungen klicken nicht, auch nicht bei gehaltenem Ton.
- [ ] B7 Auf der Seite `PARTS 4-6`: LEVEL, PAN, MUTE und SOLO wirken auf Part 5 (PD) und 6 (ORGAN) wie auf die
  anderen.

## C Zusammen

- [ ] C1 Auf Part 1 einen Akkord festhalten, Part 3 (ACID) eine Note festhalten, dann im Mixer Part 1 leiser und
  Part 3 lauter: Das Verhältnis ändert sich hörbar, nichts bricht ab.
- [ ] C2 MASTER-Regler: der Fader im Zug MASTER und der Prozentwert folgen; L und R zeigen den Pegel.
- [ ] C3 Laut: ACID mit Preset SCREAM, LEVEL +6, MASTER voll, hohe Noten mit Accent (ENV fest): `LIM` leuchtet, es
  verzerrt nicht hart (unsicher: wie es klingt, bitte notieren).

## D Speichern und Stabilität

- [ ] D1 Mixer auf beiden Seiten einstellen, SAVE, Seite neu laden und „Switch on“: alle Pegel, Pan, Mute und Solo
  wie vorher.
- [ ] D2 Fünf Minuten spielen, dabei im Mixer drehen und zwischen Parts und Seiten wechseln: keine Aussetzer, kein
  Absturz.
