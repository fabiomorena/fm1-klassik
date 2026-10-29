# Parts – Testprotokoll (Browser-Emulator)

Stand v0.6 (sechs Parts). Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/app.c`,
`firmware/klassik/dsp/engines.c`) und dem Host-Test (`firmware/klassik/tests/parts.txt`). Es geht um Funktion und
Stabilität; Klang und Design kommen zum Schluss.

Die fünf Protokolle `PARTS.md`, `MIXER.md`, `LOOPER.md`, `STEP.md` und `FX.md` gelten alle für denselben Emulator
(Stand `befb64b`, sechs Parts). Jedes beginnt mit den Werkseinstellungen, die Reihenfolge ist also egal.

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

Im Browser nicht prüfbar: MIDI-In und -Out auf den Kanälen 1–6 (der Emulator hat kein Web-MIDI; das prüft der
Host-Test), die Rechenlast auf dem Gerät.

## 0 Vorbereitung

| Was | Wie im Emulator |
| --- | --- |
| Regler drehen | Mausrad über dem Regler, oder Regler anklicken und Pfeiltasten |
| Taste oder Taster festhalten | Rechtsklick oder Shift-Klick: bleibt gedrückt, bis du noch einmal klickst |
| weiße Tasten | `Q W E R T Z U I` = F3 … F4, `A S D F G H J K` = G4 … G5 |
| OCT− / OCT+ | `Y` / `X` |
| SEL | Taster SEL (obere Tasterreihe, der zweite) anklicken, oder den Drehregler SELECT drehen (Pfeiltasten ↑ / ↓) |

- [ ] 0.1 `http://localhost:8000` mit Cmd+Shift+R neu laden, „Factory reset“ zweimal klicken (der zweite Klick
  bestätigt), dann „Switch on“. Falls die Seite nicht lädt: `python3 -m http.server 8000 -d firmware/klassik/build/emu`.

## A Parts wählen

- [ ] A1 Nach dem Start: Kopfzeile `1 VA` (orange), Sound `SAW LEAD`. Jede Kachel hat eine Part-Nummer: VA `1`
  (gefüllt), DRUMS `2`, ACID `3`, FM6 `4`, PD `5`, ORGAN `6` (umrandet).
- [ ] A2 SEL: `2 DRUMS` (lila), DRUMS umrandet und mit gefüllter `2`. Weiter mit SEL: `3 ACID`, `4 FM6`, `5 PD`,
  `6 ORGAN`, wieder `1 VA`.
- [ ] A2b Drehregler SELECT (oder Pfeiltasten ↑ / ↓): nach rechts der nächste Part, nach links der vorige, von 1 nach
  links zu 6.
- [ ] A3 Die Tasten spielen immer den Part in der Kopfzeile (auf Part 2 also Drums, auf Part 3 den ACID-Bass).
- [ ] A4 Auf Part 5 (PD) und 6 (ORGAN) spielen; Engine (K1), Sound (PRESET), Oktave und Edit-Seiten gehen wie bei den
  anderen Parts.

## B Jeder Part für sich

- [ ] B1 Oktave: auf Part 3 mit `Y` zweimal nach unten (`OCT -2`). SEL zu Part 1: `OCT +0`. Zurück zu Part 3:
  wieder `OCT -2`.
- [ ] B2 Engine: auf Part 4 (FM6) K1 eine Raste nach rechts: `4 VA`. Die VA-Kachel zeigt jetzt `1` und `4`. Part 1
  bleibt VA mit seinem Sound.
- [ ] B3 Sound: auf Part 4 mit PRESET einen anderen VA-Sound wählen. SEL zu Part 1: dort steht weiter `SAW LEAD`.
- [ ] B4 Änderung: auf Part 1 EDIT, einen Regler drehen (Name mit `*`). OCT− zurück, SEL zu Part 4 (auch VA): dort kein
  `*`, die Werte unverändert.

## C Zusammen spielen

- [ ] C1 Auf Part 1 einen Akkord festhalten (drei Tasten per Shift-Klick). SEL zu Part 2: Drums dazu spielen. Der
  Akkord klingt weiter.
- [ ] C2 Die festgehaltenen Tasten lösen (noch einmal klicken), während Part 2 gewählt ist: Der Akkord von Part 1
  verstummt, nichts bleibt hängen.
- [ ] C3 Auf Part 3 (ACID) eine Taste festhalten, SEL zu Part 4, dort Akkorde spielen: Bass und Akkorde gleichzeitig.
- [ ] C4 Viele Noten auf mehreren Parts gleichzeitig festhalten (z. B. Part 1 und 4 je sechs Tasten, dazu der
  ACID-Bass): Irgendwann verstummen die ältesten Noten (die Obergrenze). Kein Absturz, kein Dauer-Knistern.
  (unsicher: Beim Abschneiden kann es kurz knacken, das ist bekannt.)

## D Speichern

- [ ] D1 Parts einstellen (Engines, Sounds, Oktaven, auch Part 5 und 6), einen Part wählen, SAVE. Seite neu laden und
  „Switch on“: derselbe Part ist gewählt, alle sechs Parts wie eingestellt.

## E Stabilität

- [ ] E1 Fünf Minuten zwischen den Parts wechseln und spielen, dabei Engines und Presets ändern: keine Aussetzer,
  kein Absturz, keine hängenden Töne.
