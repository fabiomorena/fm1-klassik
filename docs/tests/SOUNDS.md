# SOUNDS – Testprotokoll (Browser-Emulator)

Stand v0.9 (Preset-Liste und Glyphen-Browser als GLO-Tabs LIST und GLYPHS, Mockups 37 und 38). Die erwarteten
Ergebnisse stammen aus dem Code (`firmware/klassik/app.c`) und dem Host-Test (`firmware/klassik/tests/sounds.txt`).
Es geht um Funktion und Stabilität; wie es klingt und aussieht, stimmen wir zum Schluss ab (Notizen dazu sind trotzdem
willkommen).

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

## 0 Vorbereitung

| Was | Wie im Emulator |
| --- | --- |
| Regler drehen | Mausrad über dem Regler, oder Regler anklicken und Pfeiltasten (Bild↑ / Bild↓: 5 Schritte) |
| Taste oder Taster festhalten | Rechtsklick oder Shift-Klick: bleibt gedrückt, bis du noch einmal klickst |
| weiße Tasten | `Q W E R T Z U I` = F3 … F4, `A S D F G H J K` = G4 … G5 |
| PLAY, REC, HOME | `P`, `O`, `Esc` |
| OCT− / OCT+ | `Y` / `X` |
| SEL, SELECT | SEL anklicken; SELECT: Pfeiltasten ↑ / ↓ |

Die Tastatur-Kürzel wirken nur, wenn das Panel den Fokus hat: einmal aufs Gerät klicken.

**Achtung Pfeiltasten:** Hast du zuletzt einen Regler angeklickt (zum Beispiel PRESET), drehen ↑ / ↓ diesen Regler
statt SELECT. Für SELECT den Drehregler SELECT anklicken (das dreht ihn nicht), dann ↑ / ↓.

- [ ] 0.1 `http://localhost:8000` mit **Cmd+Shift+R** neu laden, „Factory reset“ zweimal klicken (der zweite Klick
  bestätigt), dann „Switch on“. Neuer Stand? GLO, GLO, GLO zeigt den Tab `LIST` (statt wieder `MIX`).

## A GLO-Tabs

- [ ] A1 GLO öffnet den Mixer auf `MIX` (GLO leuchtet). GLO oder OCT+ geht weiter: `SEND`, `LIST`, `GLYPHS`, dann
  wieder `MIX`. Auf LIST und GLYPHS steht oben links `SOUNDS` und `1 VA`, auf MIX und SEND wie bisher `MIX PARTS 1-3`.
- [ ] A2 OCT− schließt auf jedem der vier Tabs (zurück ins Rack). GLO öffnet danach wieder auf `MIX`.
- [ ] A3 EDIT, SEQ, FX, LFO, ARP wechseln von LIST und GLYPHS zu ihren Seiten. Die Tasten spielen auf LIST und GLYPHS
  den gewählten Part.

## B LIST

- [ ] B1 Part 1 (VA, SAW LEAD), GLO bis `LIST`: rechts oben `VA`, die Zeilen `01 SAW LEAD` … mit Glyphen in Orange,
  SAW LEAD umrandet, rechts darin eine gefüllte `1` (Part 1 spielt ihn). Fußzeile `K1 ENGINE VA`, `PRESET 01/12`,
  `OCTAVE`, `MASTER`.
- [ ] B2 PRESET nach rechts: der Rahmen wandert zum nächsten Sound, er ist sofort zu hören (Taste anschlagen). Ab etwa
  dem vierten Sound bleibt der Rahmen in der Mitte und die Liste rollt; rechts ein schmaler Balken zeigt, wo man ist.
  Nach dem letzten (`12 STRINGS`) kommt wieder `01 SAW LEAD`.
- [ ] B3 K1 nach links: `ALL`. Die Liste zeigt alle 43 Presets, Engine für Engine in der Reihenfolge des Racks (VA,
  DRUMS, ACID, FM6, PD, ORGAN; die Nummern fangen je Engine bei 01 an, die Farbe der Glyphe zeigt die Engine).
  `PRESET` in der Fußzeile zählt jetzt bis 43.
- [ ] B4 Auf ALL PRESET weiterdrehen bis über `12 STRINGS` hinaus: nach STRINGS kommt `01 606` (DRUMS). Der Part
  spielt jetzt DRUMS (Kopfzeile `1 DRUMS`, die Tasten spielen Drums), in der Zeile von 606 stehen `1` und `2` (Part 2
  spielt das Kit auch). PRESET zurück: wieder VA STRINGS.
- [ ] B5 K1 von ALL nach rechts: `VA` (die Engine des Parts), weiter nach rechts: `DRUMS`, wie K1 im Rack (der Part
  spielt DRUMS mit seinem Kit). K1 zurück nach links bis `VA`.
- [ ] B6 K2, K3, K4 tun auf LIST nichts. HOME zeigt hier nicht `RESET TURN K1-K4` (es gibt nichts zurückzusetzen).
- [ ] B7 SEL oder SELECT: anderer Part; die Liste zeigt seine Engine und seinen Sound (z. B. Part 3: ACID, ACID LINE).
- [ ] B8 USER-Sound: im Rack mit PRESET `PLUCK` wählen, EDIT, K1 drehen, OCT−, SAVE (`SAVED USER`). Dann GLO bis
  `LIST`: als letzte Zeile `13 USER FROM PLUCK`, umrandet; die Fußzeile zählt bis 13.
- [ ] B9 Änderung: einen Sound mit EDIT ändern, zurück, GLO bis `LIST`: der Name in der Liste hat ein `*` wie in der
  Kopfzeile des Racks. PRESET weiter: der nächste Sound, die Änderung ist weg (wie PRESET im Rack).

Notizen:


## C GLYPHS

- [ ] C1 Part 1 auf VA `10 BRASS` stellen (PRESET), GLO bis `GLYPHS`: rechts oben `LIKE BRASS`. Neun Karten, drei je
  Reihe: zuerst BRASS mit `START` unten rechts, dann SAW LEAD, PLUCK, SOFT PAD, STRINGS, RAVE, PWM STR, ACID … (nur
  VA-Sounds, weil K1 auf `VA` steht; hast du B8 gemacht, steht USER irgendwo dazwischen).
- [ ] C2 K1 nach links auf `ALL`: jetzt sind alle Engines dabei. Nach PWM STR kommt `MARIMBA` (FM6, blau).
- [ ] C3 PRESET nach rechts: der Rahmen geht die Karten entlang, jeder Sound ist sofort zu hören. Auf MARIMBA spielt der
  Part FM6 (Kopfzeile `1 FM6`). Die Reihenfolge bleibt dabei (oben rechts weiter `LIKE BRASS`); ab der dritten Reihe
  rollt das Raster, die Reihe mit dem Rahmen bleibt in der Mitte.
- [ ] C4 GLO weiter (MIX), dann wieder bis `GLYPHS`: jetzt `LIKE MARIMBA` (bzw. der Sound, den der Part gerade spielt),
  MARIMBA mit `START` vorn, die anderen nach Ähnlichkeit zu ihm.
- [ ] C5 SEL auf GLYPHS: der nächste Part, das Raster fängt bei seinem Sound an (`LIKE` und sein Name).
- [ ] C6 Stabilität: auf ALL PRESET schnell in beide Richtungen drehen, dabei Tasten spielen: kein Hänger, kein
  hängender Ton, die Anzeige kommt mit.

Notizen:


## D Rest wie bisher

- [ ] D1 Mixer: auf MIX dreht PRESET den Sound wie bisher (ohne Liste), K1–K4 sind LEVEL, PAN, MUTE, SOLO; auf SEND
  DELAY und HALL.
- [ ] D2 SAVE, Seite neu laden, „Switch on“: Part 1 spielt den gewählten Sound wieder (auch einen Sound einer anderen
  Engine, den du auf ALL gewählt hast).

Notizen:
