# KLASSIK – Glossar

Begriffe und Kürzel vom Display, kurz erklärt. Im Handbuch (`tools/gen_manual.py`) ist das Glossar durchsuchbar, und
die Begriffe im Text verweisen hierher.

**Format:** `## Gruppe`, darin je Eintrag `### Begriff`, darunter optional eine Zeile in Kursiv mit Kapitel-Links,
anderen Schreibweisen und Werten (`*[Mixer](MANUAL.md#mixer) · auch: K1 LEVEL · Werte: L50, R50*`), dann der Text.
Der erste Satz erscheint als Kurz-Erklärung, wenn man im Handbuch auf einen Begriff zeigt. „auch“ wird im Text
verlinkt, „Werte“ nur gesucht (sonst würde z. B. BASS in „FM BASS“ verlinkt).
**Prüfung:** Jeder Name, den das Display zeigt (Simulator-Befehl `labels`, `app.c`), braucht einen Eintrag, ein
„auch“, einen Wert oder einen Platz unter „Ohne Eintrag“; die CI prüft das (`gen_manual.py --strict`).

## Bedienelemente

### SEL
*[Parts](MANUAL.md#parts)*

Taster: wählt den nächsten Part (1, 2, … 6, dann wieder 1). Tasten, K1, PRESET, OCT− / OCT+ und EDIT gehören immer
dem gewählten Part, auf jeder Seite.

### SELECT
*[Parts](MANUAL.md#parts)*

Drehregler: wählt den Part wie SEL, nach rechts den nächsten, nach links den vorigen.

### PRESET
*[Rack](MANUAL.md#rack) · auch: PRESETS*

Drehregler: wählt den Sound der Engine des gewählten Parts. Nach den Presets folgt USER, sobald die Engine einen
eigenen Sound hat; auf dem Tab LIST geht PRESET durch die Liste. Im Emulator heißt er PRESETS.

### ALGO
*[Step-Sequencer](MANUAL.md#step-sequencer) · [FM6](MANUAL.md#fm6) · auch: ALGORITHM*

Drehregler: bewegt auf dem Tab STEP den Cursor über die Steps; HOME + ALGO führt zurück zu NEW STEPS. Auf den
anderen Seiten hat er noch keine Aufgabe; im Emulator heißt er ALGORITHM. Bei FM6 heißt außerdem ein Parameter ALGO:
der Algorithmus, 1–32.

### K1–K4
*[Rack](MANUAL.md#rack) · auch: K1, K2, K3, K4, KNOB1, KNOB2, KNOB3, KNOB4*

Die vier Drehregler über den Tastern. Was sie gerade stellen, steht in der Fußzeile, je Regler ein Feld mit Balken.
HOME halten und einen von ihnen drehen setzt ihn auf seinen Grundwert zurück. Im Emulator heißen sie KNOB1–KNOB4.

### MASTER
*[Mixer](MANUAL.md#mixer)*

Drehregler für die Gesamtlautstärke, nach der Summe aller Parts und Effekte und vor dem Limiter. Im Mixer zeigt der
Zug MASTER seine Stellung und den Pegel des Ausgangs links und rechts.

### OCT−
*[Rack](MANUAL.md#rack) · auch: OCT-*

Taster: Oktave des gewählten Parts eins tiefer (bis −2). Auf den Seiten (Mixer, FX, Looper, LFO, ARP, Edit) heißt
OCT− zurück: Die Seite schließt. Bei einer Frage auf dem Display heißt OCT− nein.

### OCT+
*[Rack](MANUAL.md#rack)*

Taster: Oktave des gewählten Parts eins höher (bis +2). Auf den Seiten geht OCT+ zum nächsten Tab oder zur
nächsten Edit-Seite, wie der Taster der Seite selbst (auf ARP, mit nur einem Tab, tut er nichts). Bei einer Frage
auf dem Display heißt OCT+ ja.

### GLO
*[Mixer](MANUAL.md#mixer)*

Taster: öffnet den Mixer auf dem Tab MIX. Ist er offen, geht GLO weiter: MIX, SEND, LIST, wieder MIX.

### FX
*[Effekte](MANUAL.md#effekte)*

Taster: öffnet die Effekt-Seite auf DELAY. Ist sie offen, wechselt FX zwischen DELAY und HALL.

### SEQ
*[Looper](MANUAL.md#looper)*

Taster: öffnet die Looper-Seite. Ist sie offen, wechselt SEQ zwischen LOOP und STEP.

### PLAY
*[Looper](MANUAL.md#looper) · auch: STOP*

Taster: startet und stoppt den Looper, auf jeder Seite. HOME + PLAY fragt, ob der gewählte Part geleert werden
soll (OCT+ ja, OCT− nein). Am Gerät steht darunter STOP.

### REC
*[Looper](MANUAL.md#looper)*

Taster: Aufnahme in den Looper, auf den gewählten Part. HOME + REC nimmt die letzte Aufnahme dieses Parts zurück.

### EDIT
*[Edit-Seiten](MANUAL.md#edit-seiten)*

Taster: öffnet die Edit-Seiten der Engine des gewählten Parts, je vier Parameter auf K1–K4. Ist eine offen, geht
EDIT zur nächsten Seite.

### ENV
*[Edit-Seiten](MANUAL.md#edit-seiten) · [PD](MANUAL.md#pd)*

Taster: öffnet die Hüllkurve des gewählten Parts (VA, PD, ORGAN die Seite ENV, FM6 seine zweite Seite). Bei ACID
heißt ENV gehalten Accent (`ACC`), DRUMS hat keine Hüllkurve (`NO ENV`). ENV heißt auch die Seite mit der
Hüllkurve, und bei PD ein Parameter auf PHS: wie weit die Hüllkurve DCW mitbewegt.

### LFO
*[LFO](MANUAL.md#lfo)*

Taster: öffnet die LFO-Seite des gewählten Parts. Ist sie offen, wechselt LFO zwischen den Tabs LFO und MORE. Ein
LFO (Low Frequency Oscillator) ist eine langsame Schwingung, die einen Parameter bewegt.

### ARP
*[ARP](MANUAL.md#arp)*

Taster: öffnet die Seite des Arpeggiators für den gewählten Part. Ist sie offen, schaltet ARP LATCH an und aus.
Ein Arpeggiator spielt gehaltene Noten nacheinander.

### HOME
*[Rack](MANUAL.md#rack)*

Taster, meist gehalten: HOME + K1–K4 setzt einen Regler auf seinen Grundwert, HOME + REC nimmt die letzte Aufnahme
zurück, HOME + PLAY leert den Part (nach einer Frage), auf der Looper-Seite verdoppelt HOME + OCT+ die Schleife
und HOME + OCT− halbiert sie.

### SAVE
*[Rack](MANUAL.md#rack)*

Taster: speichert die Parts mit ihren Sounds, den Mixer und die Effekte über den Neustart hinaus; ein geänderter
Sound wird dabei zum USER-Sound seiner Engine. SAVE leuchtet, solange etwas ungespeichert ist.

## Engines

### VA
*[VA](MANUAL.md#va) · auch: ANALOG*

Virtuell-analoge Engine (Felucca ANALOG): zwei Oszillatoren durch ein Tiefpassfilter, wie ein klassischer
subtraktiver Synth. Seiten OSC, FLT, ENV, MORE.

### PD
*[PD](MANUAL.md#pd) · auch: PHASE, PHASE DIST, Phase Distortion*

Phase Distortion (Felucca PHASE): Eine Kosinuswelle wird mit verbogener Phase gelesen und bekommt so Obertöne, wie
bei den Casio CZ. DCW regelt, wie stark.

### ORGAN
*[ORGAN](MANUAL.md#organ) · auch: WHEEL, DRAWBAR*

Zugriegel-Orgel (Felucca WHEEL): neun Sinus-Fußlagen wie bei einer Tonewheel-Orgel, dazu Perkussion, Tastenklick,
Drive und Rotor.

### FM6
*[FM6](MANUAL.md#fm6) · auch: 6-OP FM*

FM-Synthese mit sechs Operatoren, nach msfa: Die Presets sind ganze DX7-Stimmen, acht Makros verschieben sie.

### DRUMS
*[DRUMS](MANUAL.md#drums) · auch: DRUM KITS*

Drum-Engine nach Plaits: 808- und 909-artige Modelle, acht Kits. Jede Taste einer Oktave ist ein Instrument.

### ACID
*[ACID](MANUAL.md#acid) · auch: MONO BASS*

Einstimmiger Bass nach Open303, im Stil der TB-303: Sägezahn oder Rechteck durch ein resonantes Filter, mit Slide
und Accent.

## Parameter

### LEVEL
*[Mixer](MANUAL.md#mixer) · [Effekte](MANUAL.md#effekte)*

Ein Pegel: im Mixer (K1 auf MIX) der eines Parts, −40 … +6 dB, darunter aus (OFF). Auf der Effekt-Seite (K4), wie
laut der Effekt zurückkommt.

### PAN
*[Mixer](MANUAL.md#mixer) · Werte: L50, R50, C*

Panorama (K2 auf MIX): wo ein Part im Stereobild steht, `L50` ganz links, `C` Mitte, `R50` ganz rechts. Die ferne
Seite wird leiser, die nahe bleibt voll.

### MUTE
*[Mixer](MANUAL.md#mixer) · auch: M*

Stummschalten (K3 auf MIX, im Zug `M`): Der Part ist still und schickt auch nichts an die Effekte; sein Zug wird
grau.

### SOLO
*[Mixer](MANUAL.md#mixer) · auch: S*

Nur dieser Part (K4 auf MIX, im Zug `S`): Solange ein Part auf Solo steht, sind alle anderen stumm, außer weiteren
Solo-Parts.

### ENGINE
*[Rack](MANUAL.md#rack) · auch: Engine, Engines*

Die Klangerzeugung eines Parts: VA, DRUMS, ACID, FM6, PD oder ORGAN. K1 im Rack und auf LIST wählt sie, in der
Reihenfolge des Racks.

### OCTAVE
*[Rack](MANUAL.md#rack)*

Die Oktave des gewählten Parts, −2 … +2 (OCT− / OCT+); im Rack in der Fußzeile.

### TIME
*[Effekte](MANUAL.md#effekte)*

K1 auf DELAY: der Abstand der Echos als Notenwert im TEMPO des Loopers (1/32 … 1/4, T triolisch, Punkt punktiert).

### FDBK
*[Effekte](MANUAL.md#effekte) · [FM6](MANUAL.md#fm6) · auch: FEEDBK*

Rückkopplung (Feedback): Ein Teil des Ausgangs geht wieder hinein. Beim Delay (K2) bestimmt sie, wie viele Echos
kommen (0–100 %); bei FM6 heißt sie FEEDBK (0–7) und macht den Operator mit Feedback rauer.

### TONE
*[Effekte](MANUAL.md#effekte) · [ORGAN](MANUAL.md#organ) · [DRUMS](MANUAL.md#drums) · auch: HAT TONE*

Klangfarbe: K3 auf DELAY (wie dunkel die Echos werden) und HALL (dunkel bis hell). HAT TONE bei DRUMS: die Farbe der
Hi-Hats. TONE heißt auch die zweite Edit-Seite von ORGAN (Perkussion, Klick, Drive, Rotor).

### PRE
*[Effekte](MANUAL.md#effekte)*

Pre-Delay (K1 auf HALL): eine Lücke von 0–100 ms vor dem Hall, damit der Ton klar bleibt.

### DECAY
*[Effekte](MANUAL.md#effekte) · [ACID](MANUAL.md#acid)*

Abklingen: K2 auf HALL, wie lange der Hall nachklingt (0–100 %, am Anfang etwa 2 s). Bei ACID, wie schnell sich das
Filter nach dem Anschlag schließt (0,2 bis 2 s).

### TEMPO
*[Looper](MANUAL.md#looper) · auch: BPM*

K1 auf LOOP: das Tempo des Loopers, 40–240 BPM (Schläge je Minute); Delay, LFO (SYNC) und Arp folgen ihm.

### LENGTH
*[Looper](MANUAL.md#looper)*

K2 auf LOOP: die Länge der Schleife, AUTO oder 1–16 Takte. AUTO heißt: Die erste Aufnahme legt sie fest.

### QUANT
*[Looper](MANUAL.md#looper)*

Quantisieren (K3 auf LOOP): wohin der Anfang einer aufgenommenen Note rückt, OFF, 1/32, 1/16, 1/8, 1/4; so liegt sie
genau im Raster.

### CLICK
*[Looper](MANUAL.md#looper) · [ORGAN](MANUAL.md#organ)*

Beim Looper (K4) das Metronom: OFF, REC (beim Vorzählen und Aufnehmen), ON (immer). Bei ORGAN der Tastenklick beim
Anschlag.

### BAR
*[Step-Sequencer](MANUAL.md#step-sequencer)*

K1 auf STEP: welcher Takt der Schleife als 16 Steps gezeigt wird.

### NOTE
*[Step-Sequencer](MANUAL.md#step-sequencer)*

K2 auf STEP: die Note neuer Steps (C1 … C7) oder des gehaltenen Steps.

### INST
*[Step-Sequencer](MANUAL.md#step-sequencer) · auch: K2 INST*

K2 auf STEP bei DRUMS: das Instrument, dessen Steps die Tasten zeigen und setzen (BD, SD, CH …).

### VEL
*[Step-Sequencer](MANUAL.md#step-sequencer)*

Velocity, die Anschlagstärke (K3 auf STEP): 1 … 127, am Anfang 100.

### LEN
*[Step-Sequencer](MANUAL.md#step-sequencer)*

K4 auf STEP: die Länge eines Steps, 1/32 … 1/1; länger als bis zur nächsten Note gleitet ACID.

### RATE
*[LFO](MANUAL.md#lfo) · [ARP](MANUAL.md#arp)*

Wie schnell: beim LFO (K1) ein Notenwert im TEMPO (SYNC ON) oder 0,1–20 Hz; beim Arp (K2) der Notenwert je
Schritt, 1/4 … 1/32.

### DEPTH
*[LFO](MANUAL.md#lfo)*

K2 auf LFO: wie stark der LFO sein Ziel bewegt, 0–100; 0 ist aus.

### WAVE
*[LFO](MANUAL.md#lfo) · [VA](MANUAL.md#va) · [PD](MANUAL.md#pd) · [ACID](MANUAL.md#acid) · Werte: SAW, SQR, TRI, SIN, PWM, PLS, DSIN, SPLS, RSAW, RTRI, RTRP, S&H*

Die Wellenform: beim LFO (K3) SIN, TRI, SAW, SQR, S&H; bei VA, PD und ACID die Form des Klangs.

| Wert | Was er ist |
| --- | --- |
| SAW | Sägezahn: hell, alle Obertöne |
| SQR | Rechteck: hohl, nur ungerade Obertöne |
| TRI | Dreieck: weich |
| SIN | Sinus: rein, ohne Obertöne |
| PWM | Rechteck mit beweglicher Pulsbreite (VA, LFO-Ziel SHAPE) |
| PLS | Puls (PD) |
| DSIN | Doppelsinus (PD) |
| SPLS | Sägezahn-Puls (PD) |
| RSAW, RTRI, RTRP | Resonanz-Wellen (PD): ein Sinus, gefenstert von Sägezahn, Dreieck oder Trapez |
| S&H | Sample and Hold (LFO): ein Zufallswert je Umdrehung |

### WAVE2
*[PD](MANUAL.md#pd)*

K2 auf PHS: eine zweite Wellenform, abwechselnd mit WAVE in jeder zweiten Schwingung; `-` heißt keine.

### DEST
*[LFO](MANUAL.md#lfo) · Werte: PITCH, SHAPE, AMP, PIT, SHP*

K4 auf LFO: wohin der LFO wirkt. PITCH (`PIT`) die Tonhöhe, FILTER (`FLT`) das Filter, bei PD DCW, SHAPE (`SHP`) die
Form, AMP die Lautstärke; welche es gibt, hängt von der Engine ab.

### SYNC
*[LFO](MANUAL.md#lfo) · [VA](MANUAL.md#va)*

Synchron: K1 auf dem LFO-Tab MORE, ON stellt RATE als Notenwert im TEMPO ein. Bei VA ist SYNC eine Wellenform
(Oszillator 2 hart an 1 synchronisiert), und DTN heißt dann SYNC: wie hoch Oszillator 2 steht.

### RETRIG
*[LFO](MANUAL.md#lfo)*

K2 auf MORE: FREE, eine Welle für alle Noten; KEY, jede Note startet die Welle neu.

### FADE
*[LFO](MANUAL.md#lfo)*

K3 auf MORE: 0–2 s, so lange braucht die Tiefe des LFO nach einer Note (verzögertes Vibrato).

### MODE
*[ARP](MANUAL.md#arp) · Werte: UP-DN, RAND, ORDER*

K1 auf ARP: in welcher Folge der Arp die Noten spielt. OFF, UP, DOWN, UP-DN (hoch und wieder runter), RAND
(zufällig), ORDER (in der Reihenfolge der Tasten).

### OCT
*[ARP](MANUAL.md#arp)*

K3 auf ARP: 1–4 Oktaven; das Muster kommt je eine Oktave höher noch einmal.

### GATE
*[ARP](MANUAL.md#arp)*

K4 auf ARP: wie lange jede Note des Arps klingt, 10–100 % des Schritts; bei ACID darüber SLIDE.

### ATK
*[Edit-Seiten](MANUAL.md#edit-seiten) · [FM6](MANUAL.md#fm6)*

Attack, das Anschwellen der Hüllkurve: wie lange ein Ton bis ganz oben braucht (VA, PD, ORGAN 0,7 ms … 1,46 s).

### DEC
*[Edit-Seiten](MANUAL.md#edit-seiten) · [DRUMS](MANUAL.md#drums) · auch: BD DEC, HAT DEC*

Decay, das Abklingen: in der Hüllkurve bis zum SUS-Pegel. Bei DRUMS der Ausklang der Bassdrum (BD DEC) und der
Hi-Hats (HAT DEC).

### SUS
*[Edit-Seiten](MANUAL.md#edit-seiten) · [FM6](MANUAL.md#fm6)*

Sustain: der Pegel, auf dem ein Ton bleibt, solange die Taste gehalten wird.

### REL
*[Edit-Seiten](MANUAL.md#edit-seiten) · [FM6](MANUAL.md#fm6)*

Release, das Ausklingen nach dem Loslassen (VA, PD, ORGAN bis −60 dB: 17 ms … 4,9 s).

### ENV>FLT
*[Edit-Seiten](MANUAL.md#edit-seiten) · auch: ENV>FILTER, ENV > FILTER*

K1 auf MORE: wie weit die Hüllkurve das Filter öffnet, −64 … 63; negativ schließt sie es.

### ENV>DCW
*[PD](MANUAL.md#pd)*

K1 auf MORE bei PD: wie weit die Hüllkurve DCW bewegt, −64 … 63.

### VOICE
*[Edit-Seiten](MANUAL.md#edit-seiten) · [ACID](MANUAL.md#acid) · Werte: POLY, MONO*

K2 auf MORE: POLY, sechs Stimmen, oder MONO, eine Stimme (eine zweite Taste spielt legato). VOICE heißt auch die
zweite Edit-Seite von ACID.

### DTN
*[VA](MANUAL.md#va) · [PD](MANUAL.md#pd) · [FM6](MANUAL.md#fm6) · auch: DTUN*

Verstimmung (Detune): bei VA Oszillator 2, bei PD die zweite Linie, in Cent. Bei FM6 heißt sie DTUN (0–7) und
verstimmt die Operatoren gegeneinander.

### MIX
*[Mixer](MANUAL.md#mixer) · [VA](MANUAL.md#va) · [PD](MANUAL.md#pd)*

Der erste Tab des Mixers: je Part LEVEL, PAN, MUTE und SOLO. Bei VA das Verhältnis von Oszillator 1 und 2; bei PD ein
Wert von LINE: die zweite Linie dazugemischt.

### NOIS
*[VA](MANUAL.md#va)*

K4 auf OSC bei VA: Rauschen dazu.

### CUT
*[VA](MANUAL.md#va) · [ACID](MANUAL.md#acid) · auch: CUTOFF*

Cutoff, die Grenzfrequenz des Filters: darüber wird es leiser. VA 30 Hz … 16 kHz, ACID (CUTOFF) 314 Hz … 2,4 kHz.

### RES
*[VA](MANUAL.md#va) · [ACID](MANUAL.md#acid) · auch: RESO, Resonanz*

Resonanz: Das Filter betont seine Grenzfrequenz; hoch klingt es pfeifend (ACID: RESO).

### DRV
*[VA](MANUAL.md#va) · [ORGAN](MANUAL.md#organ) · [DRUMS](MANUAL.md#drums) · auch: DRIVE*

Drive, Sättigung: Das Signal wird angezerrt, wärmer und rauer. VA vor dem Filter, ORGAN je Stimme, DRUMS (DRIVE)
alle Instrumente.

### KTR
*[VA](MANUAL.md#va) · auch: Keytracking*

Keytracking: wie weit die Grenzfrequenz der gespielten Note folgt; höhere Noten klingen heller.

### DCW
*[PD](MANUAL.md#pd)*

Die Tiefe der Phasenverzerrung bei PD: 0 fast ein Sinus, höher heller. Bei PD auch ein LFO-Ziel (statt FILTER).

### LINE
*[PD](MANUAL.md#pd) · Werte: RING*

Die zweite Edit-Seite von PD und dort K2: MIX mischt die zweite, verstimmte Linie dazu, RING moduliert sie ringförmig
(metallisch).

### SUB
*[VA](MANUAL.md#va) · [PD](MANUAL.md#pd) · [ORGAN](MANUAL.md#organ)*

Etwas eine Oktave tiefer: bei VA eine Wellenform (Sägezahn mit Rechteck eine Oktave tiefer, MIX heißt dann SUB), bei
PD ein Sinus eine Oktave tiefer, bei ORGAN die tiefen Zugriegel 16' und 5 1/3'.

### FOLD
*[PD](MANUAL.md#pd) · auch: Wavefolder*

Wavefolder bei PD: Die Welle wird erst gesättigt, dann gefaltet; viele neue Obertöne. 0 ist aus.

### REG
*[ORGAN](MANUAL.md#organ) · Werte: FLUTE, MELLO, HOLLW, SMOOT, 3BAR, BLUES, GOSPL, ROCK, TOPS, CLARI, REED, STRNG, CHAPL, BRITE, BASS, FULL*

Registrierung bei ORGAN: eine von 16 Einstellungen der neun Zugriegel, von FLUTE (nur 8') bis FULL (alle). Die
Liste steht im Kapitel ORGAN.

### BODY
*[ORGAN](MANUAL.md#organ)*

K3 auf BARS: die Zugriegel 8' und 4' lauter oder leiser, −8 … +8.

### TOP
*[ORGAN](MANUAL.md#organ)*

K4 auf BARS: die oberen Zugriegel, 2 2/3' bis 1', lauter oder leiser, −8 … +8.

### PERC
*[ORGAN](MANUAL.md#organ) · auch: Perkussion · Werte: 2ND, 3RD, 2SOFT, 3SOFT, 2SLOW, 3SLOW*

Perkussion bei ORGAN: ein abklingender Ton beim Anschlag, auf dem 2. (2ND) oder 3. Teilton (3RD), leiser (SOFT)
oder langsamer (SLOW).

### ROTR
*[ORGAN](MANUAL.md#organ) · auch: Rotor · Werte: SLOW, FAST*

Rotor bei ORGAN: ein rotierender Lautsprecher, OFF, SLOW oder FAST; Lautstärke und Tonhöhe schwanken leicht.

### MOD
*[FM6](MANUAL.md#fm6)*

K3 auf ALGO bei FM6: der Pegel der Modulatoren; 0 reine Sinustöne, höher heller.

### TUNE
*[ACID](MANUAL.md#acid) · [DRUMS](MANUAL.md#drums) · auch: BD TUNE, SD TUNE, TOM TUNE*

Stimmung in Halbtönen, −12 … +12: bei ACID der ganze Ton, bei DRUMS Bassdrum, Snare und Toms.

### SNAPPY
*[DRUMS](MANUAL.md#drums)*

Der Snareteppich: 0 nur der Kessel, 99 fast nur das Rauschen des Teppichs.

### ENV MOD
*[ACID](MANUAL.md#acid)*

Bei ACID: wie weit die Hüllkurve das Filter im Anschlag öffnet.

### ACCENT
*[ACID](MANUAL.md#acid)*

Bei ACID: wie stark akzentuierte Noten hervortreten (ab der nächsten Note).

### SLIDE
*[ACID](MANUAL.md#acid) · [ARP](MANUAL.md#arp) · auch: Slide*

Gleiten von einer Note zur nächsten, ohne neuen Anschlag. Bei ACID die Gleitzeit (60 wie bei der 303), beim Arp
GATE über 100 %.

## Display

### SEND
*[Mixer](MANUAL.md#mixer) · auch: Send, Sends*

Ein Abzweig: Ein Teil des Signals eines Parts geht zusätzlich an einen Effekt, das Original bleibt. So teilen sich
alle Parts einen Hall und ein Delay; im Mixer zeigt der Tab SEND, wie viel jeder an DELAY und HALL schickt.

### LIST
*[Sounds: LIST](MANUAL.md#sounds-list)*

Dritter Tab des Mixers: die Sounds als Liste mit Glyphen; PRESET wählt, K1 filtert nach Engine.

### ALL
*[Sounds: LIST](MANUAL.md#sounds-list)*

K1 auf LIST ganz links: alle Sounds aller Engines in einer Liste.

### DELAY
*[Effekte](MANUAL.md#effekte) · auch: DLY*

Das Echo: ein Ping-Pong-Delay, das erste Echo links, dann abwechselnd. Tab der Effekt-Seite und Send im Mixer (K1
auf SEND, im Zug `DLY`).

### HALL
*[Effekte](MANUAL.md#effekte)*

Der Nachhall (nach Rings' Reverb). Tab der Effekt-Seite und Send im Mixer (K2 auf SEND).

### LOOP
*[Looper](MANUAL.md#looper) · auch: Schleife*

Der erste Tab der Looper-Seite: alle Spuren, TEMPO, LENGTH, QUANT, CLICK. Die Schleife ist das, was der Looper
immer wieder abspielt.

### STEP
*[Step-Sequencer](MANUAL.md#step-sequencer) · auch: Steps*

Der zweite Tab der Looper-Seite: ein Takt als 16 Steps zu 1/16, die weißen Tasten setzen sie.

### MORE
*[LFO](MANUAL.md#lfo) · [Edit-Seiten](MANUAL.md#edit-seiten)*

Der zweite Tab des LFO (SYNC, RETRIG, FADE) und die letzte Edit-Seite von VA, PD und ORGAN (ENV > FILTER, VOICE).

### OSC
*[VA](MANUAL.md#va)*

Die erste Edit-Seite von VA: die Oszillatoren (WAVE, DTN, MIX, NOIS).

### FLT
*[VA](MANUAL.md#va) · [LFO](MANUAL.md#lfo)*

Die zweite Edit-Seite von VA: das Filter (CUT, RES, DRV, KTR) mit seiner Kurve. Auf den Kacheln des LFO: das Ziel
FILTER.

### FILTER
*[ACID](MANUAL.md#acid) · [LFO](MANUAL.md#lfo) · auch: Filter, Tiefpass*

Ein Filter nimmt Teile des Klangs weg; ein Tiefpass die Höhen über seiner Grenzfrequenz. FILTER heißt die erste
Edit-Seite von ACID und ein Ziel des LFO.

### PHS
*[PD](MANUAL.md#pd)*

Die erste Edit-Seite von PD: Welle und Phasenverzerrung (WAVE, WAVE2, DCW, ENV).

### BARS
*[ORGAN](MANUAL.md#organ)*

Die erste Edit-Seite von ORGAN: die Registrierung (REG, SUB, BODY, TOP).

### DRUM
*[DRUMS](MANUAL.md#drums)*

Die erste Edit-Seite von DRUMS: Bassdrum und Snare (BD TUNE, BD DEC, SD TUNE, SNAPPY).

### KIT
*[DRUMS](MANUAL.md#drums)*

Die zweite Edit-Seite von DRUMS: Toms, Hi-Hats und Drive (TOM TUNE, HAT DEC, HAT TONE, DRIVE).

### LIM
*[Mixer](MANUAL.md#mixer)*

Leuchtet im Zug MASTER, solange der Limiter den Ausgang leiser hält.

### PARTS
*[Mixer](MANUAL.md#mixer) · auch: PARTS 1-3, PARTS 4-6*

Kopfzeile des Mixers: welche drei Parts die Seite zeigt. Sie folgt dem gewählten Part.

### USER
*[Edit-Seiten](MANUAL.md#edit-seiten)*

Der eigene Sound einer Engine: Auf einer Edit-Seite geändert und mit SAVE gespeichert, steht er hinter den Presets.

### ACC
*[ACID](MANUAL.md#acid) · auch: Accent*

Accent: Steht in der Kopfzeile, solange bei ACID ENV gehalten ist; angeschlagene Noten sind dann lauter und mit
kräftigerem Filter-Schwung.

### Drum-Instrumente
*[DRUMS](MANUAL.md#drums) · Werte: BD, RS, SD, CP, CB, LT, MT, HT, CH, OH, CY, RD, TB, CL*

Die Kürzel der DRUMS-Instrumente, je Taste einer Oktave eines.

| Kürzel | Instrument | Taste |
| --- | --- | --- |
| BD | Bassdrum | C |
| RS | Rimshot | C# |
| SD | Snare | D |
| CP | Clap | D# |
| CB | Cowbell | E |
| LT, MT, HT | tiefe, mittlere, hohe Tom | F, G, A |
| CH | geschlossene Hi-Hat | F# |
| OH | offene Hi-Hat | A# |
| CY | Becken | B |
| RD, TB, CL | Ride-Becken, Tamburin, Claves: das eigene Instrument jedes Kits | G# |

## Begriffe

### Part
*[Parts](MANUAL.md#parts) · auch: Parts*

Eine der sechs gleichzeitig klingenden Stimmen-Gruppen, jede mit eigener Engine, eigenem Sound und eigener Oktave.
MIDI-Kanal 1–6 spielt Part 1–6.

### Stimme
*[Parts](MANUAL.md#parts) · auch: Stimmen*

Ein klingender Ton: Ein Akkord aus drei Tasten sind drei Stimmen. Alle Parts zusammen haben eine Obergrenze an
Rechenzeit; darüber endet die älteste Stimme.

### Kanalzug
*[Mixer](MANUAL.md#mixer) · auch: Zug, Kanalzüge*

Ein Part im Mixer: seine Glyphe, Nummer und Engine, darunter Fader, Pegel, Pan und die Schalter M und S.

### Rückweg
*[Effekte](MANUAL.md#effekte) · auch: Rückwege*

Was ein Effekt aus den Sends macht, kommt über den Rückweg stereo in die Summe; LEVEL auf der Effekt-Seite regelt,
wie laut.

### Limiter
*[Mixer](MANUAL.md#mixer)*

Eine automatische Lautstärke-Bremse am Ausgang: Wird es lauter als −0,5 dBFS, regelt er sofort herunter und lässt
in etwa 50 ms wieder los. So übersteuert der Ausgang nicht.

### dB
*[Mixer](MANUAL.md#mixer)*

Dezibel, das Maß für Pegel-Unterschiede: −6 dB ist etwa die halbe Amplitude, −12 dB ein Viertel, +6 dB das
Doppelte.

### dBFS
*[Mixer](MANUAL.md#mixer)*

Dezibel unter dem größten Wert, den der Ausgang darstellen kann (Full Scale): 0 dBFS ist das Maximum, −20 dBFS
deutlich darunter.

### Mono
*[Mixer](MANUAL.md#mixer)*

Ein Kanal: Die Sends nehmen die Parts mono, also vor PAN. MONO heißt auch ein Wert von VOICE: eine Stimme.

### Stereo
*[Mixer](MANUAL.md#mixer)*

Zwei Kanäle, links und rechts: Die Summe, die Rückwege der Effekte und der Ausgang sind stereo.

### Glyphe
*[Rack](MANUAL.md#rack)*

Das Bild eines Sounds: seine acht Parameter als acht Strahlen im Uhrzeigersinn ab oben, je länger, desto höher der
Wert. Sie steht überall, wo ein Sound genannt wird.

### Grundwert
*[Mixer](MANUAL.md#mixer) · auch: Marke*

Der Wert, auf den HOME + K1–K4 einen Regler setzt (Mixer: 0 dB, Mitte, aus; Edit: der Wert des Sounds). Steht der
Regler woanders, zeigt eine helle Marke im Balken der Fußzeile den Grundwert.

### Fußzeile
*[Rack](MANUAL.md#rack)*

Die unterste Zeile des Displays: was K1–K4 gerade stellen, je Regler Name, Wert und Balken.

### Kopfzeile
*[Rack](MANUAL.md#rack)*

Die oberste Zeile des Displays: Part und Engine, Glyphe und Name des Sounds, Oktave, Akku; auf den Seiten der Name
der Seite.

### Hüllkurve
*[Edit-Seiten](MANUAL.md#edit-seiten) · auch: ADSR*

Wie ein Ton über die Zeit lauter und leiser wird: Anschwellen (ATK), Abklingen (DEC), Halten (SUS), Ausklingen
nach dem Loslassen (REL).

### Operator
*[FM6](MANUAL.md#fm6) · auch: Operatoren*

Ein Sinus-Oszillator mit eigener Hüllkurve, der Baustein der FM-Synthese; FM6 hat sechs.

### Träger und Modulator
*[FM6](MANUAL.md#fm6) · auch: Träger, Modulator, Modulatoren*

Bei FM hört man die Träger; die Modulatoren verändern deren Tonhöhe so schnell, dass Obertöne entstehen.

### Zugriegel
*[ORGAN](MANUAL.md#organ) · auch: Fußlage, Fußlagen*

Bei der Orgel ein Regler für einen Teilton (eine Fußlage, 16' bis 1'), je 0–8; zusammen ergeben sie die
Registrierung.

### Vibrato
*[LFO](MANUAL.md#lfo) · auch: Tremolo*

Vibrato: die Tonhöhe schwankt (LFO auf PITCH); Tremolo: die Lautstärke schwankt (LFO auf AMP).

### Legato
*[Edit-Seiten](MANUAL.md#edit-seiten) · [ACID](MANUAL.md#acid)*

Gebunden gespielt: Die nächste Taste kommt, bevor die vorige losgelassen ist. Bei MONO und ACID gleitet oder
wechselt der Ton dann ohne neuen Anschlag.

### Choke
*[DRUMS](MANUAL.md#drums)*

Ein Instrument beendet ein anderes: Bei DRUMS beendet die geschlossene Hi-Hat eine klingende offene.

### Overdub
*[Looper](MANUAL.md#looper)*

Aufnehmen über das, was schon da ist: Jede Aufnahme im Looper kommt zu den Noten des Parts dazu.

### Vorzähler
*[Looper](MANUAL.md#looper) · auch: COUNT-IN*

Ein Takt Klick vor der Aufnahme, wenn REC bei gestopptem Looper kommt; oben zählen die Schläge herunter.

### MIDI
*[Parts](MANUAL.md#parts)*

Die Sprache, in der Instrumente Noten austauschen. Kanal 1–6 spielt Part 1–6; KLASSIK empfängt über USB und TRS und
sendet über USB.

## Ohne Eintrag (selbsterklärend)

`OFF`, `ON`, `UP`, `DOWN`
