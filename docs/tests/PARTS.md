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

- [ x] A1 Nach dem Start: Kopfzeile `1 VA` (orange), Sound `SAW LEAD`. Jede Kachel hat eine Part-Nummer: VA `1`
  (gefüllt), DRUMS `2`, ACID `3`, FM6 `4`, PD `5`, ORGAN `6` (umrandet).
- [x ] A2 SEL: `2 DRUMS` (lila), DRUMS umrandet und mit gefüllter `2`. Weiter mit SEL: `3 ACID`, `4 FM6`, `5 PD`,
  `6 ORGAN`, wieder `1 VA`.
- [x ] A2b Drehregler SELECT (oder Pfeiltasten ↑ / ↓): nach rechts der nächste Part, nach links der vorige, von 1 nach
  links zu 6.
- [x ] A3 Die Tasten spielen immer den Part in der Kopfzeile (auf Part 2 also Drums, auf Part 3 den ACID-Bass).
- [ x] A4 Auf Part 5 (PD) und 6 (ORGAN) spielen; Engine (K1), Sound (PRESET), Oktave und Edit-Seiten gehen wie bei den
  anderen Parts.

Notizen: A1: warum ist das nicht per default von 1-6 geordnet von obeen link nach unten rechts und nicht so 
kreuz und quer wie gerade - umsetzen!


## B Jeder Part für sich

- [ x] B1 Oktave: auf Part 3 mit `Y` zweimal nach unten (`OCT -2`). SEL zu Part 1: `OCT +0`. Zurück zu Part 3:
  wieder `OCT -2`.
- [x ] B2 Engine: auf Part 4 (FM6) K1 eine Raste nach rechts: `4 VA`. Die VA-Kachel zeigt jetzt `1` und `4`. Part 1
  bleibt VA mit seinem Sound.
- [x ] B3 Sound: auf Part 4 mit PRESET einen anderen VA-Sound wählen. SEL zu Part 1: dort steht weiter `SAW LEAD`.
- [ x] B4 Änderung: auf Part 1 EDIT, einen Regler drehen (Name mit `*`). OCT− zurück, SEL zu Part 4 (auch VA): dort kein
  `*`, die Werte unverändert.

## C Zusammen spielen

- [x ] C1 Auf Part 1 einen Akkord festhalten (drei Tasten per Shift-Klick). SEL zu Part 2: Drums dazu spielen. Der
  Akkord klingt weiter.
- [x ] C2 Die festgehaltenen Tasten lösen (noch einmal klicken), während Part 2 gewählt ist: Der Akkord von Part 1
  verstummt, nichts bleibt hängen.
- [ ] C3 Auf Part 3 (ACID) eine Taste festhalten, SEL zu Part 4, dort Akkorde spielen: Bass und Akkorde gleichzeitig.
- [ ] C4 Viele Noten auf mehreren Parts gleichzeitig festhalten (z. B. Part 1 und 4 je sechs Tasten, dazu der
  ACID-Bass): Irgendwann verstummen die ältesten Noten (die Obergrenze). Kein Absturz, kein Dauer-Knistern.
  (unsicher: Beim Abschneiden kann es kurz knacken, das ist bekannt.)

## D Speichern

- [x ] D1 Parts einstellen (Engines, Sounds, Oktaven, auch Part 5 und 6), einen Part wählen, SAVE. Seite neu laden und
  „Switch on“: derselbe Part ist gewählt, alle sechs Parts wie eingestellt.

## E Stabilität

- [ x] E1 Fünf Minuten zwischen den Parts wechseln und spielen, dabei Engines und Presets ändern: keine Aussetzer,
  kein Absturz, keine hängenden Töne.

## Auswertung (Claude, 29.10.2026)

- Ohne Befund: Parts wählen mit SEL und SELECT (A1–A4, auch Part 5 und 6), jeder Part für sich (B1–B4: Oktave,
  Engine, Sound, Änderung), zwei Parts zusammen (C1, C2: Akkord klingt weiter, nichts hängt), Speichern (D1), fünf
  Minuten Stabilität (E1). Die Markierungen `[ x]` und `[x ]` sind als bestanden gewertet. 0.1 ist nicht markiert;
  die anderen Punkte setzen den Start voraus, also gemacht.
- C3 und C4 sind nicht markiert, also nicht getestet. Beides prüft der Host-Test (`tests/parts.txt`: Zeile 56–66
  sechs Parts zugleich, darunter ACID und ein FM6-Akkord; ab Zeile 164 die Obergrenze, 248 > 240 Zehntel FM6-Stimmen,
  die älteste Stimme geht). Nach Gehör sind sie noch offen: Nachtest N4, N5.
- **Notiz zu A1 (Part-Nummern von 1–6 von links oben nach rechts unten, nicht kreuz und quer):** Im Simulator
  nachgestellt (`build/host/rack-parts.png`): Nach dem Start lasen sich die Nummern 4 1 5 / 6 2 3. Ursache: Die
  Kacheln standen seit dem 26.10. in der Reihenfolge FM6, VA, PD / ORGAN, DRUMS, ACID, die Parts starten seit dem
  27.10. mit VA, DRUMS, ACID, FM6, PD, ORGAN (`PART_ENGINE` in `app.c`). Du hast Variante B gewählt: Das Rack folgt
  den Parts. Umgesetzt (Mockup 29 `Rack-6`, DECISIONS.md 29.10.): Kacheln VA, DRUMS, ACID / FM6, PD, ORGAN, K1 wählt
  die Engines in dieser Reihenfolge; die Parts starten wie bisher (1 VA SAW LEAD). Gespeicherte Einstellungen bleiben
  gültig (die Engine-Nummern sind dieselben). Die K1-Schritte in sechs Host-Tests angepasst, alle 768 Prüfungen grün.

Nachtest Rack-Reihenfolge (seit 29.10.; Emulator vorher mit Cmd+Shift+R neu laden, Factory reset zweimal, „Switch on“):

- [x ] N1 Nach dem Start: oben VA `1` (gefüllt), DRUMS `2`, ACID `3`, unten FM6 `4`, PD `5`, ORGAN `6`. Kopfzeile
  `1 VA`, Sound `SAW LEAD`.
- [x ] N2 Auf Part 1 K1 Raste für Raste nach rechts: `1 DRUMS`, `1 ACID`, `1 FM6`, `1 PD`, `1 ORGAN`, wieder `1 VA`
  (die umrandete Kachel wandert mit). Von VA eine Raste nach links: `1 ORGAN`. Danach zurück auf VA.
- [ x] N3 Auf Part 4 (FM6) K1 drei Rasten nach links: `4 VA`, die VA-Kachel zeigt `1` und `4`. Part 1 bleibt VA mit
  `SAW LEAD`.
- [ x] N4 (= C3) Auf Part 3 (ACID) eine Taste festhalten, SEL zu Part 4, dort Akkorde spielen: Bass und Akkorde
  gleichzeitig.
- [x ] N5 (= C4) Viele Noten auf mehreren Parts gleichzeitig festhalten (z. B. Part 1 und 4 je sechs Tasten, dazu der
  ACID-Bass): Irgendwann verstummen die ältesten Noten (die Obergrenze). Kein Absturz, kein Dauer-Knistern.
  (unsicher: Beim Abschneiden kann es kurz knacken, das ist bekannt.)

## Auswertung Nachtest (Claude, 29.10.2026)

- N1–N5 ohne Befund (`[x ]` und `[ x]` als bestanden gewertet): Das Rack zeigt nach dem Start 1–6 von links oben nach
  rechts unten, K1 geht in dieser Reihenfolge (auch rückwärts von VA zu ORGAN), Part 4 wird mit drei Rasten VA. Bass
  und Akkorde klingen zusammen (C3), die Obergrenze schneidet die ältesten Noten ohne Absturz (C4).
- Damit ist PARTS fertig: alle Punkte bestanden, die Notiz A1 umgesetzt (`84a81bd`).
