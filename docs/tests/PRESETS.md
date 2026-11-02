# PRESETS – Testprotokoll (Browser-Emulator)

Stand v1.0 (mehr Presets: 43 → 71; der GLO-Tab GLYPHS ist seit dem 02.11. gestrichen). Die erwarteten Ergebnisse
stammen aus dem Code (`firmware/klassik/dsp/presets.c`, `eng_fm6.c`, `eng_acid.c`, `eng_drums.c`, `app.c`) und den
Host-Tests (`firmware/klassik/tests/presets.txt`, `sounds.txt`). Teil B übernimmt die Prüfungen der Liste aus dem
nicht mehr ausgefüllten SOUNDS-Protokoll (mit den Zahlen von v1.0). Sonst geht es diesmal **um den Klang**: Passt der
Charakter zum Namen, ist ein Preset viel lauter oder leiser als die anderen seiner Engine, fehlt dir etwas? Die
„gedacht als“-Spalte ist meine Absicht, gemessen habe ich nur Pegel und Glyphen (RESEARCH.md „Presets v1.0“), gehört
hat sie noch niemand.

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

In den Tabellen ersetzt du `·` durch `x`, `!` oder `-`; zu jedem `!` eine Notiz unter der Tabelle.
Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

Im Browser nicht prüfbar: Anschlagdynamik (jede Taste kommt mit Velocity 100), Rechenlast und Pegel auf dem Gerät.

## 0 Vorbereitung

| Was | Wie im Emulator |
| --- | --- |
| Regler drehen | Mausrad über dem Regler, oder Regler anklicken und Pfeiltasten (Bild↑ / Bild↓: 5 Schritte) |
| Taste oder Taster festhalten | Rechtsklick oder Shift-Klick: bleibt gedrückt, bis du noch einmal klickst |
| weiße Tasten | `Q W E R T Z U I` = F3 … F4, `A S D F G H J K` = G4 … G5 (`T` = C4) |
| schwarze Tasten | `1` … `0` und `ß` = F#3 … F#5 (`6` = F#4, `7` = G#4, `8` = A#4) |
| OCT− / OCT+ | `Y` / `X` |
| HOME | `Esc` |
| SEL, SELECT | SEL anklicken; SELECT: Pfeiltasten ↑ / ↓ |

Die Tastatur-Kürzel wirken nur, wenn das Panel den Fokus hat: einmal aufs Gerät klicken. **Achtung Pfeiltasten:** Hast
du zuletzt einen Regler angeklickt (zum Beispiel PRESET), drehen ↑ / ↓ diesen Regler statt SELECT. Für SELECT den
Drehregler SELECT anklicken (das dreht ihn nicht), dann ↑ / ↓.

- [ ] 0.1 **Ohne** Factory reset: `http://localhost:8000` mit Cmd+Shift+R neu laden, „Switch on“. Neuer Stand? GLO:
  die Tab-Zeile zeigt `MIX SEND LIST` (kein `GLYPHS` mehr). (Falls nicht: einmal privates Fenster, Cmd+Shift+N.)
- [ ] 0.2 Hast du vor dem Neubau im alten Emulator einen USER-Sound aus PLUCK gespeichert: Part 1 auf VA, GLO bis
  `LIST`. Erwartet: davor `13 SYNC LEAD`, `14 FAT BASS`, `15 POLY KEYS`, als letzte Zeile `16 USER FROM PLUCK` (dein
  USER-Sound ist USER geblieben, nicht zum neuen Preset geworden). Ohne USER-Sound: `[-]`.
- [ ] 0.3 Jetzt „Factory reset“ zweimal klicken (der zweite Klick bestätigt), dann „Switch on“. Im Rack zeigt die
  Fußzeile auf VA `PRESET 01/15`.

## A Anzahl und Reihenfolge

- [ ] A1 K1 durch die Engines, je PRESET einmal ganz herum. Fußzeile zählt: VA `/15`, DRUMS `/08`, ACID `/12`,
  FM6 `/12`, PD `/12`, ORGAN `/12`. Die neuen kommen nach den alten (VA ab 13, DRUMS ab 05, ACID ab 09, FM6 ab 09,
  PD ab 07, ORGAN ab 06); nach dem letzten wieder 01.
- [ ] A2 Jedes neue Preset hat seine eigene Glyphe (keine zwei gleich), in der Kopfzeile und in LIST dieselbe.

Notizen:


## B LIST (aus SOUNDS.md)

- [ ] B1 GLO öffnet den Mixer auf `MIX` (GLO leuchtet). GLO oder OCT+ geht weiter: `SEND`, `LIST`, dann wieder `MIX`.
  Auf LIST steht oben links `SOUNDS` und `1 VA`, auf MIX und SEND wie bisher `MIX PARTS 1-3`.
- [ ] B2 OCT− schließt auf jedem der drei Tabs (zurück ins Rack). GLO öffnet danach wieder auf `MIX`.
- [ ] B3 EDIT, SEQ, FX, LFO, ARP wechseln von LIST zu ihren Seiten. Die Tasten spielen auf LIST den gewählten Part.
- [ ] B4 Part 1 im Rack auf VA `01 SAW LEAD` (K1, PRESET), GLO bis `LIST`: rechts oben `VA`, die Zeilen `01 SAW LEAD`
  … mit Glyphen in Orange, SAW LEAD umrandet, rechts darin eine gefüllte `1` (Part 1 spielt ihn). Fußzeile
  `K1 ENGINE VA`, `PRESET 01/15`, `OCTAVE`, `MASTER`.
- [ ] B5 PRESET nach rechts: der Rahmen wandert zum nächsten Sound, er ist sofort zu hören (Taste anschlagen). Ab etwa
  dem vierten Sound bleibt der Rahmen in der Mitte und die Liste rollt; rechts ein schmaler Balken zeigt, wo man ist.
  Nach dem letzten (`15 POLY KEYS`) kommt wieder `01 SAW LEAD`.
- [ ] B6 K1 nach links: `ALL`. Die Liste zeigt alle 71 Presets, Engine für Engine in der Reihenfolge des Racks (VA,
  DRUMS, ACID, FM6, PD, ORGAN; die Nummern fangen je Engine bei 01 an, die Farbe der Glyphe zeigt die Engine).
  `PRESET` in der Fußzeile zählt jetzt bis 71.
- [ ] B7 Auf ALL PRESET weiterdrehen bis über `15 POLY KEYS` hinaus: danach kommt `01 606` (DRUMS). Der Part spielt
  jetzt DRUMS (Kopfzeile `1 DRUMS`, die Tasten spielen Drums), in der Zeile von 606 stehen `1` und `2` (Part 2 spielt
  das Kit auch). PRESET zurück: wieder VA POLY KEYS.
- [ ] B8 K1 von ALL nach rechts: `VA` (die Engine des Parts), weiter nach rechts: `DRUMS`, wie K1 im Rack (der Part
  spielt DRUMS mit seinem Kit). K1 zurück nach links bis `VA`.
- [ ] B9 K2, K3, K4 tun auf LIST nichts. HOME zeigt hier nicht `RESET TURN K1-K4` (es gibt nichts zurückzusetzen).
- [ ] B10 SEL oder SELECT: anderer Part; die Liste zeigt seine Engine und seinen Sound (z. B. Part 3: ACID, ACID LINE).
- [ ] B11 USER-Sound: im Rack auf Part 1 VA mit PRESET `PLUCK` wählen, EDIT, K1 drehen, OCT−, SAVE (`SAVED USER`).
  Dann GLO bis `LIST`: als letzte Zeile `16 USER FROM PLUCK`, umrandet; die Fußzeile zählt bis 16.
- [ ] B12 Änderung: einen Sound mit EDIT ändern, zurück, GLO bis `LIST`: der Name in der Liste hat ein `*` wie in der
  Kopfzeile des Racks. PRESET weiter: der nächste Sound, die Änderung ist weg (wie PRESET im Rack).
- [ ] B13 Mixer: auf MIX dreht PRESET den Sound wie bisher (ohne Liste), K1–K4 sind LEVEL, PAN, MUTE, SOLO; auf SEND
  DELAY und HALL.
- [ ] B14 Stabilität: auf LIST mit ALL PRESET schnell in beide Richtungen drehen, dabei Tasten spielen: kein Hänger,
  kein hängender Ton, die Anzeige kommt mit.
- [ ] B15 SAVE, Seite neu laden, „Switch on“: Part 1 spielt den gewählten Sound wieder (auch einen Sound einer anderen
  Engine, den du auf ALL gewählt hast).

Notizen:


## C Anhören

Je Preset: einzelne Töne in der Mitte (`T` = C4), einen Akkord (wo mehrstimmig), dann ganz tief (OCT− zweimal) und
ganz hoch (OCT+ zweimal). Vergleich „Lautstärke“: mit zwei, drei alten Presets derselben Engine.

### VA (ab 13)

| Preset | gedacht als | Charakter passt | Lautstärke ähnlich wie die anderen | Ausklang | tiefste Lage ok | höchste Lage ok |
| --- | --- | --- | --- | --- | --- | --- |
| SYNC LEAD | schneidender Sync-Lead, einstimmig | · | · | · | · | · |
| FAT BASS | Sägezahn mit Sub-Oktave, einstimmig, Filter öffnet beim Anschlag | · | · | · | · | · |
| POLY KEYS | mehrstimmige Synth-Keys (Rechteck), gehalten etwas leiser | · | · | · | · | · |

Notizen:


### PD (ab 07)

| Preset | gedacht als | Charakter passt | Lautstärke ähnlich wie die anderen | Ausklang | tiefste Lage ok | höchste Lage ok |
| --- | --- | --- | --- | --- | --- | --- |
| BASS | runder Bass mit Sub, Anschlag heller, einstimmig | · | · | · | · | · |
| E.PIANO | weiches E-Piano, heller Anschlag, verklingt | · | · | · | · | · |
| SWEEP PAD | Fläche, langsamer Einsatz, öffnet sich, leicht schwebend | · | · | · | · | · |
| RESO LEAD | resonanter Lead, etwas angezerrt, schwebend, einstimmig | · | · | · | · | · |
| FOLD BASS | knurriger Bass (Wavefolder), einstimmig | · | · | · | · | · |
| CHIME | metallisch-glockig, verklingt | · | · | · | · | · |

Notizen:


### ORGAN (ab 06)

| Preset | gedacht als | Charakter passt | Lautstärke ähnlich wie die anderen | Ausklang | tiefste Lage ok | höchste Lage ok |
| --- | --- | --- | --- | --- | --- | --- |
| BLUES | Blues-Registrierung, Perkussion, angezerrt, schneller Rotor | · | · | · | · | · |
| BALLAD | weich, langsamer Rotor, sanfter Ein- und Ausklang | · | · | · | · | · |
| MELLOW | hohl, langsame Perkussion, langsamer Rotor | · | · | · | · | · |
| CHAPEL | kirchenartig weich, kein Rotor, langsamer Ein- und Ausklang | · | · | · | · | · |
| REED | Zungenregister, bläserartig, angezerrt | · | · | · | · | · |
| CLARINET | hohl wie eine Klarinette (nur ungerade Obertöne) | · | · | · | · | · |
| BRIGHT | hell, Perkussion, Klick, schneller Rotor | · | · | · | · | · |

Notizen:


### FM6 (ab 09)

| Preset | gedacht als | Charakter passt | Lautstärke ähnlich wie die anderen | Ausklang | tiefste Lage ok | höchste Lage ok |
| --- | --- | --- | --- | --- | --- | --- |
| HARPSI | Cembalo: hell gezupft, verklingt beim Halten, endet beim Loslassen kurz | · | · | · | · | · |
| VIBES | Vibraphon: weicher Schlag, langer Ausklang (Tremolo: LFO AMP) | · | · | · | · | · |
| STRINGS | Streicher: langsamer Bogen, spätes Vibrato | · | · | · | · | · |
| FLUTE | Flöte mit Atemgeräusch und Vibrato | · | · | · | · | · |

Notizen:


### ACID (ab 09; Accent: ENV halten beim Anschlag)

| Preset | gedacht als | Charakter passt | Lautstärke ähnlich wie die anderen | mit Accent ok | tiefste Lage ok | höchste Lage ok |
| --- | --- | --- | --- | --- | --- | --- |
| DARK | dunkel, Filter weit zu, lange Decay | · | · | · | · | · |
| OPEN | offen und hell, wenig Quaken | · | · | · | · | · |
| HOLLOW | Säge und Rechteck gemischt, hohl | · | · | · | · | · |
| CHIRP | kurzer, zwitschernder Anschlag (viel ENV MOD, kurze Decay) | · | · | · | · | · |

Notizen:


### DRUMS (ab 05; Tasten: `T` BD, `Z` SD, `6` CH, `8` OH, `7` G#)

| Kit | gedacht als | Charakter passt | BD ok | SD ok | Hats ok | Lautstärke ähnlich wie 808/909 |
| --- | --- | --- | --- | --- | --- | --- |
| TECH | 909-artig, lange Bassdrum, angezerrt; G# Ride | · | · | · | · | · |
| BOOM | 808-artig, tief und lang (Sub-Boom), dunkle Hats; G# Claves | · | · | · | · | · |
| LOFI | dumpf, angezerrt, Hats und Becken in groben Stufen; G# Tamburin | · | · | · | · | · |
| TIGHT | kurz und hoch, knackige Snare, kurze helle Hats; G# Claves | · | · | · | · | · |

Notizen:


## D Was fehlt?

- [ ] D1 Gibt es Klänge, die du bei einer Engine vermisst (z. B. „ein richtiger Pad bei FM6“)? Je Engine sind noch
  Plätze frei: VA 0, PD 3, ORGAN 3, FM6 3, ACID 3, DRUMS 7.
- [ ] D2 Gibt es alte Presets, die neben den neuen überflüssig wirken (Kandidaten zum Streichen im Feinschliff)?

Notizen:

