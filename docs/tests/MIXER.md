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
Engine!) statt SELECT. Für SELECT den Drehregler SELECT anklicken (das dreht ihn nicht), dann ↑ / ↓.

Für Pan Kopfhörer nehmen (ob der Lautsprecher des FM-1 stereo ist, ist unbekannt).

## 0 Vorbereitung

- [ ] 0.1 `http://localhost:8000` mit Cmd+Shift+R neu laden, „Factory reset“ zweimal klicken (der zweite Klick
  bestätigt), dann „Switch on“. HOME im Emulator: `Esc`; Taster festhalten: Shift-Klick.

## A Öffnen, Seiten, schließen

- [x ] A1 GLO: Kopfzeile `MIX PARTS 1-3`, GLO leuchtet; darunter die Tabs `MIX` (hell) und `SEND`, rechts zwei kleine
  Marken, die linke hell (Seite 1). Vier Züge: `1 VA`, `2 DRUMS`, `3 ACID`, `MASTER`, die Namen ganz lesbar. Der Zug
  von Part 1 ist umrandet. Fußzeile `K1 LEVEL 0 dB`, `K2 PAN C`, `K3 MUTE OFF`, `K4 SOLO OFF`.
- [x ] A2 SEL: Der Rahmen wandert zu Part 2, die Fußzeile zeigt seine Werte. Die Tasten spielen den umrandeten Part.
- [ x] A2b SEL bis Part 4: die Seite wechselt zu `PARTS 4-6` mit `4 FM6`, `5 PD`, `6 ORGAN`, die rechte Marke ist
  hell. SEL von 6 auf 1: zurück zur ersten Seite. Mit SELECT (Pfeiltasten ↑ / ↓) genauso, auch rückwärts.
- [x ] A3 GLO oder OCT+ wechselt zwischen `MIX` und `SEND`; OCT− schließt den Mixer (zurück zum Rack); EDIT im Mixer
  öffnet stattdessen die Edit-Seite.

Notizen: A1: lass uns delay und hall anzeige entfernen, damit etwas mehr platz entsteht auf der mix seite
Wir werden bestimmt noch weiter Effekte hinzufügen und die würde man dann ja auch nicht mehr sehen.


## B Ein Part

- [ x] B1 K1 LEVEL: leiser / lauter (bis +6 dB), der Fader folgt; ganz unten `OFF`, der Part ist still.
- [ x] B2 K2 PAN (Kopfhörer): nach links nur noch links, nach rechts nur rechts, Mitte beide.
- [ x] B3 K3 MUTE nach rechts: `M` leuchtet, der Part ist still und grau; nach links wieder da.
- [x ] B4 K4 SOLO auf einem Part: alle anderen Parts verstummen (grau), auch die auf der anderen Seite. Solo auf einem
  zweiten Part: beide klingen.
- [ x] B5 LEVEL nicht auf 0 dB oder PAN nicht in der Mitte: Eine helle Marke im Balken der Fußzeile zeigt 0 dB bzw.
  die Mitte. HOME (`Esc`) halten und K1–K4 drehen: zurück auf 0 dB, Mitte, aus; die Marken sind weg.
- [x ] B6 Die Änderungen klicken nicht, auch nicht bei gehaltenem Ton.
- [ x] B7 Auf der Seite `PARTS 4-6`: LEVEL, PAN, MUTE und SOLO wirken auf Part 5 (PD) und 6 (ORGAN) wie auf die
  anderen.

## C Zusammen

- [ c] C1 Auf Part 1 einen Akkord festhalten, Part 3 (ACID) eine Note festhalten, dann im Mixer Part 1 leiser und
  Part 3 lauter: Das Verhältnis ändert sich hörbar, nichts bricht ab.
- [c ] C2 MASTER-Regler: der Fader im Zug MASTER und der Prozentwert folgen; L und R zeigen den Pegel.
- [ x] C3 Laut: ACID mit Preset SCREAM, LEVEL +6, MASTER voll, hohe Noten mit Accent (ENV fest): `LIM` leuchtet, es
  verzerrt nicht hart (unsicher: wie es klingt, bitte notieren).

## D Speichern und Stabilität

- [x ] D1 Mixer auf beiden Seiten einstellen, SAVE, Seite neu laden und „Switch on“: alle Pegel, Pan, Mute und Solo
  wie vorher.
- [x ] D2 Fünf Minuten spielen, dabei im Mixer drehen und zwischen Parts und Seiten wechseln: keine Aussetzer, kein
  Absturz.

## Auswertung (Claude, 29.10.2026)

- Ohne Befund: Öffnen und Seiten (A1–A3, auch `PARTS 4-6` und SELECT rückwärts), LEVEL, PAN, MUTE, SOLO über beide
  Seiten, Marken und HOME-Reset, kein Klicken, Part 5 und 6, Limiter, Speichern, fünf Minuten Stabilität.
- C1 und C2 sind mit `c` markiert: als bestanden gewertet (c liegt neben x). Falls du etwas anderes meintest, sag
  Bescheid. 0.1 ist nicht markiert; die anderen Punkte setzen den Start voraus, also gemacht.
- **Notiz zu A1 (DLY und HALL aus dem Tab MIX nehmen, damit Platz frei wird und spätere Effekte nicht fehlen):** Eine
  Design-Änderung. Vorschlag als Mockups 28a (MIX ohne Sends, längere Fader) und 28b (SEND mit einem senkrechten
  Regler je Effekt, Platz für bis zu vier auf K1–K4) in `docs/design/`; Fragen im Chat, Code erst nach deinem „passt
  so“.

Nachtest neuer Mixer (seit 29.10., Mockups 28a/28b; Emulator vorher mit Cmd+Shift+R neu laden):

- [x ] N1 GLO, Tab MIX: je Part Glyphe und Name, ein langer, fetter Fader (Griff mit runden Ecken), darunter Pegel,
  PAN und M S; **keine** DLY/HALL. MASTER: Fader, L/R-Pegel, Prozent, LIM. Alles aus A und B geht wie vorher.
- [ x] N2 GLO (oder OCT+), Tab SEND: je Part zwei senkrechte Fader `DLY` und `HALL` mit Wert darunter, beim umrandeten
  Part hell. K1 und K2 bewegen sie, SEL wandert, Seite 4–6 genauso. MASTER zeigt grau die Rückwege und LIM.

## Auswertung Nachtest (Claude, 29.10.2026)

- N1 und N2 ohne Befund (`[x ]` und `[ x]` als bestanden gewertet): Tab MIX ohne Sends mit den längeren, runden
  Fadern, Tab SEND mit einem Fader je Effekt. Damit ist MIXER fertig.
