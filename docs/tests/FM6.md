# FM6 – Testprotokoll (Browser-Emulator)

Stand v0.2, Commit `1a9eb14`. Die erwarteten Ergebnisse stammen aus dem Code (`firmware/klassik/app.c`,
`firmware/klassik/dsp/eng_fm6.c`, `firmware/klassik/dsp/engines.c`) und den Host-Tests
(`firmware/klassik/tests/fm6.txt`). Wo die Erwartung unsicher ist, steht „(unsicher)“: Dann einfach notieren, was
passiert.

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

In den Tabellen ersetzt du `·` durch `x`, `!` oder `-`; zu jedem `!` eine Notiz unter der Tabelle.
Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

Im Browser nicht prüfbar: Anschlagdynamik (jede Taste kommt mit Velocity 100, `base/web/emu/emu_web.c:86`), MIDI-In
(der Emulator hat kein Web-MIDI; MIDI prüfen die Host-Tests), Rechenlast und Pegel auf dem Gerät.

## 0 Vorbereitung

| Was | Wie im Emulator |
| --- | --- |
| Regler drehen | Mausrad über dem Regler, oder Regler anklicken und Pfeiltasten (Bild↑ / Bild↓: 5 Schritte) |
| Taste oder Knopf festhalten | Rechtsklick oder Shift-Klick: bleibt gedrückt, bis du noch einmal klickst |
| weiße Tasten | `Q W E R T Z U I` = F3 … F4, `A S D F G H J K` = G4 … G5 (`T` = C4) |
| schwarze Tasten | `1` … `0` und `ß` = F#3 … F#5 |
| OCT− / OCT+ | `Y` / `X` |
| PLAY, REC, HOME | `P`, `O`, `Esc` |

Die Tastatur-Kürzel wirken nur, wenn das Panel den Fokus hat: einmal aufs Gerät klicken.

- [x ] 0.1 `http://localhost:8000` öffnen. Erwartet: das FM-1 mit dem Knopf „Switch on“.
  (Falls die Seite nicht lädt, im Terminal: `python3 -m http.server 8000 -d firmware/klassik/build/emu`)
- [ x] 0.2 „Factory reset“ zweimal drücken (beim ersten Mal steht dort „Press again to forget everything“), die Seite
  lädt neu; dann „Switch on“. Erwartet: Status „On.“ (steht dort eine andere Abtastrate, bitte notieren); Rack mit
  VA umrandet, Fußzeile `K1 ENGINE VA`, `PRESET 01/…`, `OCTAVE +0`, `MASTER 68%`.

## A Rack: Auswahl

- [ x] A1 K1 eine Raste nach links. Erwartet: FM6-Kachel umrandet, Kopfzeile hellblauer Chip `FM6`, Glyphe,
  `E.PIANO`; Fußzeile `K1 ENGINE FM6`, `PRESET 01/08`.
- [ x] A2 K1 schrittweise nach rechts: VA → PD → ORGAN → wieder FM6. Nach links von FM6 aus: ORGAN.
- [x ] A3 Nach einem Engine- oder Preset-Wechsel leuchtet SAVE (es gibt Ungespeichertes).
- [x ] A4 Mit FM6: PRESET nach rechts durch alle acht: E.PIANO, GLASS BELL, FM BASS, BRASS, MARIMBA, FM ORGAN,
  WARM PAD, CLAV, dann wieder E.PIANO. Nach links von E.PIANO aus: CLAV. Fußzeile zählt `01/08` … `08/08`.
- [ x] A5 Jedes Preset hat eine eigene Glyphe (keine zwei gleich), auf der FM6-Kachel und in der Kopfzeile dieselbe.
- [x ] A6 Im Rack tun K2, K3, K4, SELECT und ALGORITHM noch nichts (kein Wechsel, kein Absturz).
- [x ] A7 Die Knöpfe ohne Funktion zeigen kurz „… SOON“: FX, SEL, ENV, LFO, GLO, HOME, ARP, SEQ, PLAY, REC.

Notizen:




## B Spielen

- [x ] B1 Mit E.PIANO alle 27 Tasten nacheinander: jede klingt, die Tonhöhe steigt von links nach rechts, keine bleibt
  hängen.
- [x ] B2 OCT− / OCT+: Kopfzeile und Fußzeile zeigen −2 … +2, darüber und darunter geht es nicht weiter.
- [x ] B3 Eine Taste halten (Rechtsklick), Oktave wechseln, Taste loslassen: Der Ton endet (kein hängender Ton).
- [x ] B4 Sechs Tasten gleichzeitig halten: alle sechs klingen. Eine siebte dazu: Der älteste Ton verstummt, der neue
  klingt; nach dem Loslassen hängt nichts.
- [x ] B5 Eine Taste schnell wiederholt anschlagen: Jeder Anschlag ist zu hören, kein Knacken (unsicher).
- [ x] B6 Einen Ton halten, mit K1 die Engine wechseln: Der Ton bricht sofort ab (ein Knacken dabei bitte notieren).
- [x ] B7 Einen Ton halten, mit PRESET das Preset wechseln: Der Ton klingt weiter und endet beim Loslassen (wie er
  dabei klingt, ist offen: bitte notieren).
- [ x] B8 MASTER ganz zu: Stille. Ganz auf: deutlich lauter, ohne Verzerrung bei einem einzelnen Ton.
- [x ] B9 MASTER ganz auf, sechs Töne mit E.PIANO, dann mit FM ORGAN: verzerrt es? (unsicher; der Pegel wird erst auf
  dem Gerät eingemessen)

Notizen:
B7: klingt normal ab
B9: keine Verzerrung zu hören


## C Presets

Je Preset: einen Ton, einen Akkord (drei Tasten) halten und loslassen; einmal bei Oktave −2 (`Q`) und einmal bei +2
(`K`) anspielen. In die Spalten `x`, `!` oder `-`.

| Preset | gedacht als | Charakter passt | Lautstärke ähnlich wie die anderen | Ausklang | tiefste Lage ok | höchste Lage ok |
| --- | --- | --- | --- | --- | --- | --- |
| E.PIANO | DX-E-Piano, glockiger Anschlag, klingt gehalten aus | · | · | · | · | · |
| GLASS BELL | helle Glocke, klingt nach dem Loslassen Sekunden nach | · | · | · | · | · |
| FM BASS | kurzer, knackiger Bass, hält beim Halten | · | · | · | · | · |
| BRASS | Bläser, kleiner Ansatz von unten, spätes Vibrato | · | · | · | · | · |
| MARIMBA | kurz, holzig, verklingt auch gehalten | · | · | · | · | · |
| FM ORGAN | Orgel, gleichmäßig, solange gehalten | · | · | · | · | · |
| WARM PAD | weiche Fläche, langsamer Ein- und Ausklang, Vibrato | · | · | · | · | · |
| CLAV | Clavinet, kurz und hell, verklingt auch gehalten | · | · | · | · | · |

Notizen:

Generell, wenn ich extrem schnell über die Klaviatur gehe, knackt Glass BEll, FM Organ z.B.
Ansonsten passen die sounds erste einmal, alle angehört!

## D Edit-Seite ALGO

Mit E.PIANO beginnen.

- [x ] D1 EDIT: Seite `ALGO` (`1/2` oben rechts), große Glyphe, die Namen ALGO, FEEDBK, MOD, DTUN leuchten hellblau;
  Fußzeile `K1 ALGO 5`, `K2 FEEDBK 3`, `K3 MOD 76`, `K4 DTUN 2`.
- [ x] D2 Nach der ersten Änderung: `*` hinter dem Namen (`E.PIANO*`), SAVE leuchtet. Den Wert zurückdrehen: Das `*`
  bleibt (erst SAVE oder ein Preset-Wechsel nimmt es weg).
- [x ] D3 Glyphe: Der Strahl des gedrehten Parameters wird sofort länger oder kürzer; im Rack und in der Kopfzeile
  dieselbe Glyphe.
- [x ] D4 Grenzen: ALGO bleibt bei 1 und 32 stehen, FEEDBK bei 0 und 7, MOD bei 0 und 99, DTUN bei 0 und 7 (kein
  Sprung ans andere Ende).

### ALGO 1–32, live

Einen Ton festhalten (Rechtsklick), ALGO Raste für Raste von 1 bis 32 drehen. Erwartet: Der Klang ändert sich sofort,
auch am gehaltenen Ton; kein Wert ist stumm (OP1 ist in allen 32 Algorithmen ein Träger). Die Lautstärke springt
mit: mehr Träger klingen meist lauter (ALGO 32: alle sechs).

- [ x] D5 ALGO 1–32 mit E.PIANO. Auffällige Werte (stumm, Knacken, Verzerrung): …
- [x ] D6 ALGO 1–32 mit einem zweiten Preset deiner Wahl (welches: …). Auffällige Werte: …

### FEEDBK, MOD, DTUN

- [ x] D7 FEEDBK 0 → 7 an einem gehaltenen Ton: wirkt sofort; 0 sauber, 7 rau bis rauschig.
- [ x] D8 MOD wirkt erst ab der nächsten Note: drehen, neu anschlagen. MOD 0: reine Sinustöne (dumpf, ohne Obertöne),
  solange ALGO der des Presets ist; 99: sehr hell bis harsch.
- [ x] D9 MOD bei FM ORGAN: ändert nichts, solange ALGO 32 ist (dort sind alle sechs Operatoren Träger, MOD verschiebt
  nur Modulatoren). Mit ALGO 1 wirkt MOD auch bei FM ORGAN; dann klingt es schon bei MOD 0 hell, weil MOD relativ
  zum Preset verschiebt und FM ORGANs Operatoren laut sind (so gebaut; sag, ob dich das stört).
- [x ] D10 DTUN ab der nächsten Note: 0 keine Schwebung, 7 deutliche Schwebung (wie Chorus).

## E Edit-Seite ENV

- [x ] E1 EDIT noch einmal (oder OCT+): Seite `ENV` (`2/2`), Fußzeile `K1 ATK 5`, `K2 DEC 95`, `K3 SUS 0`, `K4 REL 48`
  (bei E.PIANO). EDIT oder OCT+ noch einmal: zurück zu `ALGO`.
- [x ] E2 Alle vier wirken erst ab der nächsten Note: An einem gehaltenen Ton ändert Drehen nichts.
- [ x] E3 ATK 0: sofort da; 99: schwillt sehr langsam an (über deutlich mehr als eine Sekunde, unsicher wie lange).
- [ x] E4 DEC 0: fällt nach dem Anschlag schnell ab; 99: fällt sehr langsam.
- [x ] E5 SUS 0: der Ton klingt auch gehalten ganz aus; 99: er hält, solange die Taste gedrückt ist.
- [x ] E6 REL 0: endet beim Loslassen sofort; 99: klingt sehr lange nach (viele Sekunden).
- [x ] E7 REL 99 und schnell viele Töne: Neue Töne nehmen den ältesten ihre Stimme (B4), nichts hängt, kein Knacken
  (unsicher).
- [ x] E8 Grenzen: ATK, DEC, SUS, REL bleiben bei 0 und 99 stehen.

## F Edit-Seiten: Bedienung

- [x ] F1 OCT− auf einer Edit-Seite: zurück ins Rack (die Oktave ändert sich dabei nicht).
- [x ] F2 Auf einer Edit-Seite wählt K1 keine Engine (K1 ist dort ALGO bzw. ATK).
- [x ] F3 PRESET auf einer Edit-Seite: lädt das nächste Preset, die Seite bleibt offen, die Werte springen auf die des
  Presets, das `*` ist weg (die Änderung ist verloren).

## G Extremwerte je Preset

Je Preset jeden Parameter einmal ganz auf Minimum und einmal auf Maximum, jeweils neu anschlagen (ALGO und FEEDBK an
einem gehaltenen Ton). Gesucht: Stille, Knacken, Verzerrung, hängende Töne, Abstürze. Zwischen zwei Parametern das
Preset mit PRESET neu laden (einmal weiter und zurück), damit sich die Änderungen nicht addieren. Bei FM ORGAN ändert
MOD nichts (D9): Das ist dort ein `x`.

| Preset | ALGO | FEEDBK | MOD | DTUN | ATK | DEC | SUS | REL |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| E.PIANO | · | · | · | · | · | · | · | · |
| GLASS BELL | · | · | · | · | · | · | · | · |
| FM BASS | · | · | · | · | · | · | · | · |
| BRASS | · | · | · | · | · | · | · | · |
| MARIMBA | · | · | · | · | · | · | · | · |
| FM ORGAN | · | · | · | · | · | · | · | · |
| WARM PAD | · | · | · | · | · | · | · | · |
| CLAV | · | · | · | · | · | · | · | · |

Notizen:

Alles wie erwartet soweit!

## H Speichern

- [ x] H1 Mit FM6 einen Wert ändern, SAVE: kurz `SAVED USER`, das `*` ist weg, SAVE erlischt; der Name ist `USER`,
  im Rack zeigt die Fußzeile `PRESET USER`.
- [x ] H2 PRESET nach rechts: nach CLAV kommt USER, danach E.PIANO.
- [x ] H3 Seite neu laden (Cmd+R), „Switch on“: FM6 ist gewählt, `USER` mit deinen Werten (auf den Edit-Seiten
  nachsehen).
- [ x] H4 Ein anderes Preset ändern, SAVE: Der neue Sound ersetzt den alten USER (es gibt nur einen je Engine).
- [ x] H5 USER selbst ändern und SAVE: bleibt USER, mit den neuen Werten.
- [x ] H6 Ohne Änderung SAVE: kurz `SAVED` (ohne USER), SAVE erlischt. Nach Neuladen sind Engine, Preset und Oktave so,
  wie sie beim SAVE waren.
- [x ] H7 FM6 ändern (nicht speichern), mit OCT− ins Rack, K1 auf VA und zurück auf FM6: das `*` und die Werte sind
  noch da.
- [ x] H8 Ändern, nicht speichern, Seite neu laden: Die Änderung ist weg, es gilt der letzte SAVE.
- [x ] H9 „Factory reset“ (zweimal): USER ist weg, es startet wieder VA.

## I Dauerbetrieb

- [x ] I1 Fünf Minuten frei spielen, dabei Presets und Regler wechseln: keine Aussetzer, kein Knistern, kein Absturz.
  (Der Browser rechnet anders als das Gerät; Aussetzer hier sagen wenig über das Gerät.)
- [x ] I2 „Mute“ schaltet den Ton stumm und wieder ein.

## Freie Notizen

Was dir sonst auffällt: Klang, Bedienung, Anzeige, Wünsche.

Soweit bin ich erst einmal zufrieden. Klang war bis auf ein, zwei Fälle wo ich wirklich schnell gespielt habe, mit
knacken zu hören, aber das war ein extremfall

## Auswertung (Claude, 26.10.2026)

- 50 von 50 Punkten wie erwartet; Tabellen C und G laut Notizen ohne Befund.
- Knacken bei sehr schnellem Spiel: im Simulator nachgemessen. Ursache: Ein noch klingender Ton, wieder angeschlagen,
  bekam dieselbe Stimme, und msfa setzt beim Start Phase und Hüllkurve aller Operatoren auf 0. Behoben mit
  `fm_note_retrigger` (`firmware/klassik/dsp/fm6/msfa.c`): Operatoren über −42 dB machen weiter, wo sie sind.
  Gemessen: C4 16-mal wiederholt, GLASS BELL und FM ORGAN, je 15 Sprünge vorher, 0 nachher (alle Zahlen in
  `docs/RESEARCH.md`, Abschnitt FM6).
- Offen: im Browser noch einmal schnell spielen. Bleibt ein Knacken, liegt es eher am Emulator (er rechnet das
  Zeichnen des Bildschirms im Audio-Thread), unsicher.
