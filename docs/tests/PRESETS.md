# PRESETS – Testprotokoll (Browser-Emulator)

Stand v1.0 (mehr Presets: 43 → 71). Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/dsp/presets.c`,
`eng_fm6.c`, `eng_acid.c`, `eng_drums.c`, `app.c`) und dem Host-Test (`firmware/klassik/tests/presets.txt`). Hier
geht es diesmal **um den Klang**: Passt der Charakter zum Namen, ist ein Preset viel lauter oder leiser als die
anderen seiner Engine, fehlt dir etwas? Die „gedacht als“-Spalte ist meine Absicht, gemessen habe ich nur Pegel und
Glyphen (RESEARCH.md „Presets v1.0“), gehört hat sie noch niemand.

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

- [ ] 0.1 **Ohne** Factory reset: `http://localhost:8000` mit Cmd+Shift+R neu laden, „Switch on“. Hast du im
  SOUNDS-Test (B8) einen USER-Sound gespeichert: Part 1 auf VA, GLO bis `LIST`. Erwartet: davor `13 SYNC LEAD`,
  `14 FAT BASS`, `15 POLY KEYS`, als letzte Zeile `16 USER FROM PLUCK` (dein USER-Sound ist USER geblieben, nicht
  zum neuen Preset geworden). Ohne USER-Sound: `[-]`.
- [ ] 0.2 Jetzt „Factory reset“ zweimal klicken, dann „Switch on“. Im Rack zeigt die Fußzeile auf VA `PRESET 01/15`.

## A Anzahl und Reihenfolge

- [ ] A1 K1 durch die Engines, je PRESET einmal ganz herum. Fußzeile zählt: VA `/15`, DRUMS `/08`, ACID `/12`,
  FM6 `/12`, PD `/12`, ORGAN `/12`. Die neuen kommen nach den alten (VA ab 13, DRUMS ab 05, ACID ab 09, FM6 ab 09,
  PD ab 07, ORGAN ab 06); nach dem letzten wieder 01.
- [ ] A2 GLO bis `LIST`, K1 auf `ALL`: `PRESET` zählt bis 71.
- [ ] A3 Jedes neue Preset hat seine eigene Glyphe (keine zwei gleich), in der Kopfzeile und in LIST dieselbe.

Notizen:


## B Anhören

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


## C Was fehlt?

- [ ] C1 Gibt es Klänge, die du bei einer Engine vermisst (z. B. „ein richtiger Pad bei FM6“)? Je Engine sind noch
  Plätze frei: VA 0, PD 3, ORGAN 3, FM6 3, ACID 3, DRUMS 7.
- [ ] C2 Gibt es alte Presets, die neben den neuen überflüssig wirken (Kandidaten zum Streichen im Feinschliff)?

Notizen:

