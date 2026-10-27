# ACID – Testprotokoll (Browser-Emulator)

Stand v0.3 (Teil ACID). Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/dsp/eng_acid.c`,
`firmware/klassik/dsp/acid/acid.c`, `firmware/klassik/dsp/engines.c`, `firmware/klassik/app.c`) und dem Host-Test
(`firmware/klassik/tests/acid.txt`). Wo die Erwartung unsicher ist, steht „(unsicher)“: Dann einfach notieren, was
passiert.

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

Im Browser nicht prüfbar: MIDI-In (Velocity ab 100 = Accent), Rechenlast und Pegel auf dem Gerät.

## 0 Vorbereitung

| Was | Wie im Emulator |
| --- | --- |
| Regler drehen | Mausrad über dem Regler, oder Regler anklicken und Pfeiltasten (Bild↑ / Bild↓: 5 Schritte) |
| Taste oder Taster festhalten | Rechtsklick oder Shift-Klick: bleibt gedrückt, bis du noch einmal klickst |
| weiße Tasten | `Q W E R T Z U I` = F3 … F4, `A S D F G H J K` = G4 … G5 (`T` = C4) |
| schwarze Tasten | `1` … `0` und `ß` = F#3 … F#5 |
| OCT− / OCT+ | `Y` / `X` |

Die Kürzel wirken nur, wenn das Panel den Fokus hat: einmal aufs Gerät klicken. Für Bass die Oktave mit `Y` zweimal
nach unten (`OCT -2`: `T` ist dann C2).

- [ ] 0.1 Der Server auf `http://localhost:8000` läuft und zeigt den neuen Stand (von Claude gebaut). Falls die
  Seite nicht lädt, im Terminal: `python3 -m http.server 8000 -d firmware/klassik/build/emu`; falls ACID noch
  „SOON“ zeigt: `.venv/bin/python base/tools/build.py firmware/klassik --web` (letzte Zeile `emu … (… B wasm)`).
- [ ] 0.2 `http://localhost:8000` mit Cmd+Shift+R neu laden, „Factory reset“ zweimal, dann „Switch on“.
  Erwartet: Status „On.“; Rack mit VA umrandet. (Die alten Einstellungen sind ohnehin verworfen: neues
  Speicherformat `KLS5`.)

## A Rack

- [ ] A1 Die sechste Kachel heißt ACID (gelb, „MONO BASS“, Glyphe, „ACID LINE“); keine Kachel zeigt mehr „SOON“.
- [ ] A2 K1 von VA aus vier Rasten nach rechts: ACID ist gewählt, Kopfzeile gelber Chip `ACID`, Glyphe,
  `ACID LINE`; Fußzeile `K1 ENGINE ACID`, `PRESET 01/08`. Eine Raste weiter: wieder FM6.
- [ ] A3 PRESET durch alle acht: ACID LINE, SQUELCH, DEEP SAW, SQUARE, RUBBER, SCREAM, PLUCK, SLOW SLIDE, dann
  wieder ACID LINE; jedes hat eine eigene Glyphe.

## B Spielen (ACID LINE, `OCT -2`)

- [ ] B1 `T` kurz: ein 303-artiger Basston, hell im Anschlag, dann schnell dunkler.
- [ ] B2 `T` gehalten: Der Ton wird über etwa eine Sekunde leiser und dunkler (die 303 hat kein Sustain), beim
  Loslassen ist er sofort weg.
- [ ] B3 Eine Stimme: `T` halten, dann `U` dazu: Es klingt nur ein Ton, nie zwei.
- [ ] B4 Slide: `T` halten, `U` dazu drücken: Der Ton gleitet zu E hinauf, ohne neuen Anschlag. `U` loslassen,
  `T` noch gehalten: Er gleitet zurück zu C.
- [ ] B5 `T` halten, `U` dazu, dann `T` loslassen (`U` bleibt): Nichts ändert sich, E klingt weiter.
- [ ] B6 Accent: ENV per Shift-Klick festhalten. ENV leuchtet, in der Kopfzeile erscheint `ACC` (gelb umrandet).
  Noten spielen: lauter, mit kräftigerem „Wah“ im Anschlag. ENV noch einmal klicken: `ACC` ist weg, die Noten
  wieder normal.
- [ ] B7 Mit ENV fest und `T`/`U` gebunden (wie B4): Auch die gleitende Note ist akzentuiert, so macht es Open303.
  Weil beim Gleiten die Hüllkurve nicht neu startet, wirkt der Accent dort schwächer als bei einem neuen Anschlag
  (unsicher: notiere, ob du ihn hörst).
- [ ] B8 ENV auf einer anderen Engine (z. B. VA): Die Kopfzeile zeigt kurz „ENV SOON“, kein `ACC`.

## C Seite FILTER (EDIT)

- [ ] C1 EDIT öffnet FILTER: Glyphe groß, Strahlen TUNE, CUTOFF, RESO, ENV MOD oben rechts in Gelb, Fußzeile
  `K1 TUNE 0`, `K2 CUTOFF 35`, `K3 RESO 75`, `K4 ENV MOD 60` (wie das Mockup `Edit-ACID`).
- [ ] C2 Eine Note halten, CUTOFF (K2) drehen: heller / dunkler, sofort.
- [ ] C3 RESO (K3): mehr Quäken und Pfeifen nach oben; bei 99 fast ein Pfeifton.
- [ ] C4 ENV MOD (K4): bei 0 kaum „Wah“ im Anschlag, bei 99 ein starker Filter-Schwung.
- [ ] C5 TUNE (K1) bei gehaltener Note: Der Ton gleitet zur neuen Höhe, −12 bis +12 Halbtöne.

## D Seite VOICE (EDIT noch einmal oder OCT+)

- [ ] D1 VOICE: DECAY, ACCENT, WAVE, SLIDE leuchten; Fußzeile `K1 DECAY 40`, `K2 ACCENT 70`, `K3 WAVE 0`,
  `K4 SLIDE 60`.
- [ ] D2 DECAY (K1): kurzer bzw. langer Filter-Schwung, ab der nächsten Note.
- [ ] D3 ACCENT (K2), ENV fest: bei 0 klingen akzentuierte Noten fast wie normale, bei 99 deutlich lauter und
  heller; ab der nächsten Note.
- [ ] D4 WAVE (K3): 0 Säge (schnarrend), 99 Rechteck (hohl), dazwischen gemischt; sofort.
- [ ] D5 SLIDE (K4) mit gebundenen Noten (B4): bei 0 springt der Ton ohne Gleiten, bei 99 gleitet er langsam
  (etwa eine Drittelsekunde).
- [ ] D6 OCT− zurück ins Rack; der Soundname hat ein `*` (bearbeitet). SAVE: Meldung „SAVED USER“, PRESET zeigt
  `USER`.

## E Presets nach Gehör

Je Preset eine kurze Linie in `OCT -2` mit ein paar gebundenen Noten und ein paar Accents. Notiere, ob der Name
passt und was dir fehlt.

- [ ] E1 ACID LINE
- [ ] E2 SQUELCH
- [ ] E3 DEEP SAW
- [ ] E4 SQUARE
- [ ] E5 RUBBER
- [ ] E6 SCREAM
- [ ] E7 PLUCK
- [ ] E8 SLOW SLIDE

## F Klang

- [ ] F1 `firmware/klassik/build/host/acid-line.wav` (vom Host-Test, ACID LINE, sechzehn Sechzehntel mit Slides
  und Accents): klingt wie eine 303-Linie? (unsicher: Geschmackssache, bitte notieren, was dir auffällt)
- [ ] F2 Hohe Lagen (`OCT +1`, `+2`) mit SCREAM und Accent bei vollem MASTER: Es darf verzerren. Die
  Resonanzspitze trifft dort die Obertöne, gemessen bis zu 5,6 dB über Vollaussteuerung. Wie stark stört es?

## G Stabilität

- [ ] G1 Fünf Minuten Bass spielen, dabei Presets und Regler wechseln: keine Aussetzer, kein Knistern, kein
  Absturz.
- [ ] G2 Eine Note halten und mit K1 (im Rack) die Engine wechseln: Kein Ton bleibt hängen; zurück auf ACID spielt
  wieder normal.
