# KLASSIK – Bedienung (v0.6: Parts, Mixer, Looper, Step-Sequencer, Effekte)

## Parts

KLASSIK hat vier Parts, die gleichzeitig klingen, jeder mit eigener Engine, eigenem Sound und eigener Oktave. Am
Anfang: Part 1 VA, 2 DRUMS, 3 ACID, 4 FM6. Die Kopfzeile zeigt den gewählten Part und seine Engine („1 VA“); auf
jeder Kachel stehen die Parts, die diese Engine spielen (der gewählte gefüllt, die anderen umrandet).

- **SEL** (der Taster) wählt den nächsten Part (1, 2, 3, 4, 1); der Drehregler **SELECT** ebenso, nach rechts den
  nächsten, nach links den vorigen. Tasten, K1, PRESET, OCT− / OCT+ und EDIT gehören immer dem
  gewählten Part. Eine Taste, die beim Wechsel noch gehalten ist, klingt auf ihrem Part weiter, bis sie losgelassen
  wird.
- **MIDI:** Kanal 1–4 spielt Part 1–4, die anderen Kanäle den gewählten Part. Die Tasten senden auf dem Kanal ihres
  Parts.
- Jeder Part merkt sich je Engine seinen Sound und seine Änderungen. Zwei Parts können dieselbe Engine mit
  verschiedenen Sounds spielen.
- Alle Parts zusammen haben eine Obergrenze an Rechenzeit. Wird sie überschritten, endet die älteste Stimme, zuerst
  eine schon losgelassene. Eine ACID-Stimme kostet so viel wie gut sieben FM6-Stimmen. Die Grenze ist geschätzt und
  wird mit einem Gerät eingemessen.

## Rack

| Bedienelement | Was es tut |
| --- | --- |
| Tasten | spielen den gewählten Part (Anschlagdynamik; ACID: Accent mit ENV), senden die Noten über USB-MIDI |
| SEL, SELECT | nächster Part (SELECT nach links: voriger) |
| K1 | Engine des Parts wählen, in der Reihenfolge des Racks: FM6, VA (ANALOG), PD (PHASE), ORGAN (WHEEL), DRUMS, ACID |
| PRESET | Sound der Engine wählen; nach den Presets folgt USER, sobald die Engine einen eigenen Sound hat |
| OCT- / OCT+ | Oktave des Parts −2 … +2 |
| EDIT | Edit-Seiten der Engine öffnen |
| GLO | Mixer öffnen (dort wechselt GLO zwischen MIX und SEND, OCT− schließt ihn) |
| FX | Effekt-Seite öffnen (dort wechselt FX zwischen DELAY und HALL, OCT− schließt sie) |
| SEQ | Looper-Seite öffnen (dort wechselt SEQ zwischen LOOP und STEP, OCT− schließt sie) |
| PLAY, REC | Looper starten und stoppen, aufnehmen (auf jeder Seite, siehe Looper) |
| HOME + REC, HOME + PLAY | letzte Aufnahme zurücknehmen, Part leeren (siehe Looper) |
| ENV | bei ACID: gehalten, sind die angeschlagenen Noten akzentuiert (`ACC` in der Kopfzeile) |
| SAVE | die Parts speichern: den gewählten, je Part Engine, Sounds und Oktave, Mixer und Effekte (leuchtet, solange etwas ungespeichert ist) |
| MASTER | Lautstärke |

MIDI-In über USB und TRS spielt ebenfalls.

## Mixer

GLO öffnet den Mixer auf dem Tab MIX (GLO leuchtet); GLO oder OCT+ wechselt zum Tab SEND und zurück (wie SEQ auf
der Looper-Seite), OCT− schließt ihn, EDIT, SEQ und FX wechseln zu ihren Seiten. Für jeden Part gibt es einen
Kanalzug: Glyphe, Part und Engine, Fader, Pegel, Pan, die Sends DLY und HALL als Balken, M (Mute) und S (Solo). Der
Zug des gewählten Parts ist umrandet, SEL wählt den nächsten. Farbig ist, was die Regler des Tabs gerade drehen (MIX:
die Fader, SEND: die Sends). Die Tasten spielen weiter.

**Tab MIX:**

| Regler | Was er tut |
| --- | --- |
| K1 LEVEL | Pegel des Parts, −40 … +6 dB in 1-dB-Schritten, darunter aus (OFF); am Anfang 0 dB |
| K2 PAN | links (L50) … Mitte (C) … rechts (R50): die ferne Seite wird leiser, die nahe bleibt; in der Mitte beide voll |
| K3 MUTE | nach rechts an, nach links aus |
| K4 SOLO | nach rechts an, nach links aus; ist ein Part auf Solo, sind alle anderen stumm (außer weiteren Solo-Parts) |
| HOME + K1–K4 | zurück auf 0 dB, Mitte, aus; steht LEVEL oder PAN woanders, zeigt eine helle Marke im Balken, wo 0 dB bzw. die Mitte ist |

**Tab SEND:**

| Regler | Was er tut |
| --- | --- |
| K1 DELAY | wie viel der Part ans Delay schickt, 0–100; am Anfang 0 |
| K2 HALL | wie viel er an den Hall schickt, 0–100; am Anfang 0 |
| K3, K4 | frei (später für weitere Effekte) |
| HOME + K1, K2 | zurück auf 0; eine helle Marke am Anfang des Balkens, solange der Send woanders steht |

Die Sends kommen nach LEVEL, MUTE und SOLO, aber vor PAN: Die Effekte bekommen die Parts mono. Ein stummer Part
schickt also auch nichts an die Effekte; sein Hall und seine Echos klingen aus. 50 ist etwa −12 dB, 100 der volle
Pegel des Parts.

Ein stummer Part erscheint grau. MASTER zeigt den MASTER-Regler, den Pegel des Ausgangs für links und rechts, wie
laut die Effekte zurückkommen (DLY, HALL: LEVEL der Effekt-Seite) und `LIM`. Der Limiter hält den Ausgang unter −0,5 dBFS: Er greift sofort ein, wenn es lauter würde, und lässt in etwa
50 ms wieder los. Solange er arbeitet, leuchtet `LIM`. Änderungen gleiten in wenigen Millisekunden, ohne Knacken.
SAVE speichert den Mixer mit.

## Effekte

Zwei Send-Effekte für alle Parts: **DELAY**, ein Ping-Pong-Delay im Takt, und **HALL**, der Hall von Mutable
Instruments Rings. Was jeder Part an sie schickt, steht im Mixer (Tab SEND); ihr Ausgang geht stereo in den Master,
vor den Limiter.

FX öffnet die Effekt-Seite auf dem Tab DELAY (FX leuchtet); FX oder OCT+ wechselt zu HALL und zurück, OCT− schließt
sie, EDIT, GLO und SEQ wechseln zu ihren Seiten. Oben ein Bild des Effekts, darunter für jeden Part, wie viel er an
diesen Effekt schickt (der gewählte umrandet, ein stummer grau mit `M`). Die Tasten spielen weiter.

**DELAY:** Das erste Echo kommt links, das nächste rechts, dann wieder links, jedes um FDBK leiser und mit TONE
dunkler. Das Bild zeigt die Note und ihre Echos (links nach oben, rechts nach unten), darüber steht, was gespielt wird
(`1/8. = 375 MS`), rechts in der Tab-Zeile das Tempo. Die Zeit folgt dem TEMPO des Loopers, auch wenn er nicht läuft.

| Regler | Was er tut |
| --- | --- |
| K1 TIME | Notenwert: 1/32, 1/16T, 1/16, 1/8T, 1/16., 1/8, 1/4T, 1/8., 1/4 (T = triolisch, Punkt = punktiert); am Anfang 1/8. Länger als 0,5 s (langsames Tempo): die halbe Zeit, wenn nötig noch einmal halbiert, so bleibt es im Takt (bei 40 BPM wird aus 1/4 ein 1/16) |
| K2 FDBK | Rückkopplung 0–100 % (wie viele Echos); am Anfang 35 % |
| K3 TONE | 0–100 %: wie dunkel die Echos werden (100 = offen); am Anfang 70 % |
| K4 LEVEL | wie laut das Delay zurückkommt, −40 … +6 dB, darunter aus; am Anfang 0 dB (bei Send 100 so laut wie der Part) |
| HOME + K1–K4 | zurück auf 1/8., 35 %, 70 %, 0 dB, mit Marke im Balken |

Eine neue Zeit blendet in 23 ms über. Das Delay hält höchstens 0,5 s je Seite.

**HALL:** Rings' Hall mit festen Räumen (keine Raumgröße), stereo. Das Bild zeigt die Note, die Lücke von PRE und den
Nachhall.

| Regler | Was er tut |
| --- | --- |
| K1 PRE | Verzögerung vor dem Hall, 0–100 ms; am Anfang 20 ms |
| K2 DECAY | wie lange er nachklingt, 0–100 %; am Anfang 50 % (etwa 2 s) |
| K3 TONE | 0–100 %: dunkel bis hell; am Anfang 60 % |
| K4 LEVEL | wie laut der Hall zurückkommt, −40 … +6 dB, darunter aus; am Anfang 0 dB |
| HOME + K1–K4 | zurück auf 20 ms, 50 %, 60 %, 0 dB, mit Marke im Balken |

Schickt kein Part an einen Effekt, schaltet er sich ab, sobald seine Echos oder sein Nachhall verklungen sind (er
kostet dann keine Rechenzeit). SAVE speichert die Effekte und die Sends mit.

## Looper

Der Looper nimmt Noten auf, keinen Klang: was die Tasten und MIDI dem gewählten Part spielen. Alle vier Parts teilen
sich eine Schleife (Länge und Tempo); jeder Part hat darin seine eigene Spur. Die Loops liegen im RAM und sind nach
dem Ausschalten weg; Tempo, QUANT und CLICK auch (sie stehen dann wieder auf 120 BPM, 1/16, REC).

| Bedienelement | Was es tut |
| --- | --- |
| PLAY | Schleife von vorn starten, noch einmal: stoppen (auf jeder Seite); leuchtet grün, beim Aufnehmen rot. Leer und LENGTH AUTO: `EMPTY REC FIRST` |
| REC | den gewählten Part aufnehmen, über das, was er schon hat (Overdub), bis REC noch einmal; leuchtet dabei. Gestoppt: erst ein Takt Vorzähler mit Klick (CLICK OFF: sofort) |
| SEL, SELECT | beim Aufnehmen wandert die Aufnahme mit zum nächsten Part |
| HOME + REC | die letzte Aufnahme zurücknehmen, egal welcher Part sie gemacht hat (die Meldung nennt ihn: `UNDO 2 DRUMS`); noch einmal: wieder da (`REDO`). Eine Stufe |
| HOME + PLAY | den gewählten Part leeren; fragt erst (`CLEAR 2 DRUMS? OCT+`): OCT+ leert, OCT− behält. HOME + REC holt ihn zurück |

**Die erste Aufnahme** (alle Spuren leer, LENGTH AUTO) legt die Länge fest: REC beendet sie, auf ganze Takte gerundet
(der nächste, 1–16), und die Schleife spielt weiter; PLAY beendet sie und stoppt. Was nach dem Ende gespielt wurde,
rückt an den Anfang. Nach 16 Takten steht die Länge von selbst fest.

**Die Looper-Seite** (SEQ öffnet sie auf dem Tab LOOP; SEQ oder OCT+ wechselt zum Tab STEP, wie EDIT und OCT+ auf den
Edit-Seiten; OCT− zurück; EDIT, GLO und FX wechseln zu ihren Seiten): je Part eine Spur mit Glyphe, Name
und seinen Noten (höher = weiter oben, DRUMS eine Zeile je Instrument), die Taktstriche, die weiße Linie, wo gerade
gespielt wird. Die Spur des gewählten Parts ist umrandet, `REC` steht an der, die aufnimmt; ein stummer Part (Mixer)
ist grau. Oben die Taktnummern, die gerade spielende hell. Die Tasten spielen weiter.

| Regler | Was er tut |
| --- | --- |
| K1 TEMPO | 40–240 BPM; am Anfang 120 |
| K2 LENGTH | AUTO oder 1–16 Takte. Kürzer blendet die Takte danach aus (sie bleiben erhalten), länger hängt leere an. AUTO nur, solange alles leer ist; nicht während der ersten Aufnahme |
| K3 QUANT | OFF, 1/32, 1/16, 1/8, 1/4: wohin der Anfang einer aufgenommenen Note rückt (ihr Ende rückt gleich weit, die Länge bleibt); am Anfang 1/16 |
| K4 CLICK | OFF, REC (beim Vorzählen und Aufnehmen), ON (immer, wenn die Schleife läuft); am Anfang REC. Auf der Eins höher |
| HOME + K1–K4 | zurück auf 120 BPM, die Länge der ersten Aufnahme (alles leer: AUTO), 1/16, REC; eine helle Marke im Balken zeigt den Wert, solange der Regler woanders steht |

**Kopfzeile:** Auf der Looper-Seite steht links, was der Looper tut (`REC 2 DRUMS`, `PLAY`, `STOP`, `COUNT-IN`,
`EMPTY`). Rechts, auf jeder Seite: ein roter Punkt beim Aufnehmen (ein grünes Dreieck beim Spielen) und Takt.Schlag
(`3.2`); beim Vorzählen die Schläge, die noch kommen (`4` … `1`).

Hinweise: Der Looper beendet beim Abspielen nur Noten, die er selbst begonnen hat; spielt er aber genau die Note, die
du gerade hältst, endet mit seiner auch deine. Eine Spur
fasst 1024 Noten; ist sie voll, kommen keine neuen dazu (`FULL 2 DRUMS`).

## Step-Sequencer

Der Tab STEP der Looper-Seite (SEQ oder OCT+) zeigt einen Takt der Schleife als 16 Steps zu 1/16, eine Reihe je Part.
Es sind dieselben Noten wie im Looper: Ein Step ist gesetzt, wo eine Note des Parts in ihm beginnt (bis 1/32 davor
oder danach). Live Eingespieltes erscheint also als Steps und lässt sich hier nachbearbeiten. Der Step, der gerade
spielt, leuchtet in allen Reihen auf, seine Nummer oben auch.

| Bedienelement | Was es tut |
| --- | --- |
| weiße Tasten | die 16 Steps des gewählten Parts (F3 = Step 1 … G5 = Step 16). Ein leerer Step wird beim Drücken gesetzt (mit K2–K4), ein gesetzter beim Loslassen gelöscht, außer es wurde dabei gedreht. Ihre Lichter zeigen die gesetzten Steps; das Licht des Steps, der gerade spielt, kippt (Lauflicht) |
| schwarze Tasten | nichts (auf STEP spielen die Tasten nicht) |
| K1 BAR | welcher Takt gezeigt wird |
| K2 NOTE | Note neuer Steps (C1 … C7; am Anfang C4). DRUMS: K2 INST wählt das Instrument (BD, RS, SD, CP, CB, LT, CH, MT, das eigene des Kits auf G#, HT, OH, CY), die Tasten zeigen und schalten dann nur dessen Steps |
| K3 VEL | Anschlag neuer Steps (1 … 127; am Anfang 100) |
| K4 LEN | Länge neuer Steps: 1/32, 1/16, 1/8, 3/16, 1/4, 3/8, 1/2, 3/4, 1/1 (am Anfang 1/16). Eine Note endet kurz vor ihrer Länge; länger als bis zur nächsten Note gleitet ACID (Slide) |
| Taste halten + K2–K4 | Note, Anschlag, Länge dieses Steps (gehalten: weiß umrandet, unten seine Werte); mehrere gehaltene ändern sich zusammen |
| HOME + K2–K4 | zurück auf C in der Oktave des Parts (DRUMS: BD), 100, 1/16, mit Marke im Balken; mit gehaltener Taste für diesen Step |

Ist die Schleife leer (LENGTH AUTO), macht der erste Step sie einen Takt lang. Während der ersten Aufnahme setzt STEP
nichts (`STEP AFTER TAKE`). Ein Step auf dem Part der letzten Aufnahme hebt HOME + REC für diese auf (sie ließe sich
nicht mehr allein zurücknehmen). Unten steht beim gewählten Part der gehaltene Step (`STEP 09 HELD`) oder, was neue
Steps bekommen (`NEW STEPS`); bei DRUMS die Reihe des Instruments und die Instrumente des Kits (ein Punkt: hat Steps
in diesem Takt).

## Edit-Seiten

Jede Engine hat zwei Seiten mit je vier Parametern: FM6 ALGO und ENV, VA OSC und FLT, PD PHS und LINE, ORGAN BARS
und TONE, DRUMS DRUM und KIT, ACID FILTER und VOICE. In der Mitte steht die Glyphe des Sounds groß; die Namen an den Strahlen der vier
Parameter auf K1–K4 leuchten in der Farbe der Engine. Bei DRUMS stehen dort stattdessen die Tasten einer Oktave
(siehe DRUMS).

| Bedienelement | Was es tut |
| --- | --- |
| K1–K4 | die vier Parameter der Seite; Klang und Glyphen ändern sich sofort (bei FM6 einige erst ab der nächsten Note, siehe unten) |
| EDIT oder OCT+ | die andere Seite |
| OCT- | zurück zum Rack |
| HOME + K1–K4 | HOME halten und einen Regler drehen: Er springt auf den Wert seines Sounds zurück (des Presets oder des USER-Sounds) |
| PRESET | anderen Sound laden (eine ungespeicherte Änderung geht dabei verloren) |
| SAVE | wie im Rack, siehe unten |

Ein geänderter, noch nicht gespeicherter Sound hat ein `*` hinter dem Namen, und SAVE leuchtet. Eine helle Marke im
Balken eines Reglers zeigt, wo sein Sound ihn hat; steht jeder Regler wieder dort (mit HOME oder von Hand), sind
`*` und Marken weg. SAVE macht ihn zum
USER-Sound seiner Engine und speichert alles über den Neustart hinaus. Jede Engine hat einen USER-Sound für alle
Parts; ein neuer ersetzt den alten (haben zwei Parts dieselbe Engine geändert, gilt der gewählte Part). Er behält Hüllkurve und MONO des Presets, aus dem er entstanden ist. Wer zwischendurch die Engine
wechselt, behält die Änderungen bis zum Ausschalten; ein SAVE speichert die geänderten Sounds aller Engines.

## FM6

Sechs Operatoren wie bei einem DX7, mit dessen 32 Algorithmen, Hüllkurven, Pitch-Hüllkurve und LFO. Jedes Preset ist
eine vollständige DX7-Stimme; die acht Parameter beschreiben sie und verschieben sie beim Drehen als Ganzes, sodass
ihr Charakter erhalten bleibt.

| Seite | Parameter | Was er tut |
| --- | --- | --- |
| ALGO | ALGO 1–32 | der Algorithmus: welche Operatoren man hört (Träger) und welche sie modulieren |
| ALGO | FEEDBK 0–7 | die Rückkopplung des Operators mit Feedback; höher klingt rauer |
| ALGO | MOD 0–99 | der Pegel des lautesten Modulators; alle Modulatoren folgen. 0: reine Sinustöne, höher: heller |
| ALGO | DTUN 0–7 | verstimmt die Operatoren gegeneinander (Schwebung); 0: keine |
| ENV | ATK 0–99 | Anschwellen: 0 sofort, 99 sehr langsam |
| ENV | DEC 0–99 | Abklingen nach dem Anschlag: 0 schnell, 99 sehr langsam |
| ENV | SUS 0–99 | der Pegel, solange die Taste gehalten wird; 0: der Ton klingt auch gehalten ganz aus |
| ENV | REL 0–99 | Ausklingen nach dem Loslassen: 0 sofort, 99 sehr lang |

ALGO und FEEDBK wirken sofort, auch auf gehaltene Töne; die anderen sechs ab der nächsten Note. Die Anschlagdynamik
steckt in den Presets: Bei E.PIANO, CLAV und MARIMBA klingt ein härterer Anschlag heller, FM ORGAN reagiert nicht
darauf.

Presets: E.PIANO, GLASS BELL, FM BASS, BRASS, MARIMBA, FM ORGAN, WARM PAD, CLAV. Es sind eigene Klänge, keine
Werksklänge des DX7. GLASS BELL und WARM PAD klingen nach dem Loslassen mehrere Sekunden nach.

## DRUMS

Vier Kits im Stil der Roland-Drumcomputer TR-606, TR-707, TR-808 und TR-909, mit PRESET gewählt („606“ … „909“).
Alle Klänge werden berechnet, es sind keine Samples aus den Geräten. Der 707 und die Becken der 909 waren im
Original Samples; ihre Kits hier sind Nachbildungen im Stil, nicht dieselben Klänge.

Jede Taste spielt ein Instrument, in jeder Oktave gleich:

| Taste | C | C# | D | D# | E | F | F# | G | G# | A | A# | B |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Instrument | BD | RS | SD | CP | CB | LT | CH | MT | je Kit | HT | OH | CY |

BD Bassdrum, RS Rimshot, SD Snare, CP Clap, CB Cowbell, LT / MT / HT tiefe, mittlere, hohe Tom, CH geschlossene,
OH offene Hi-Hat, CY Becken. G# hat jedes Kit für sich: 606 und 909 RD (Ride-Becken), 707 TB (Tamburin), 808 CL
(Claves). BD, RS, SD, CP, CH und OH liegen dort, wo General MIDI sie hat: Ein Drum-Sequencer per
MIDI trifft sie.

- Der Anschlag ist der Accent: härter gespielt klingt lauter und kräftiger.
- Jedes Instrument hat eine Stimme, wie bei den Originalen: nochmal angeschlagen, klingt es von dort weiter, wo es
  gerade ist.
- Die geschlossene Hi-Hat beendet eine klingende offene.
- Das Loslassen einer Taste ändert nichts: Jeder Schlag klingt von selbst aus.

Die Oktave (OCT− / OCT+) ändert bei DRUMS nichts am Klang: Jede Oktave spielt dieselben Instrumente.

Die Edit-Seite zeigt die Tasten einer Oktave als Pads, jede mit ihrem Instrument. Lila umrandet sind die, die die
Regler der Seite ändern; das zuletzt gespielte leuchtet kurz.

| Seite | Parameter | Was er tut |
| --- | --- | --- |
| DRUM | BD TUNE −12 … +12 | Stimmung der Bassdrum in Halbtönen |
| DRUM | BD DEC 0–99 | Ausklang der Bassdrum: 0 kurz, 99 lang dröhnend |
| DRUM | SD TUNE −12 … +12 | Stimmung der Snare in Halbtönen |
| DRUM | SNAPPY 0–99 | Anteil des Snareteppichs (Rauschen): 0 nur der Kessel, 99 fast nur der Teppich |
| KIT | TOM TUNE −12 … +12 | Stimmung aller drei Toms in Halbtönen |
| KIT | HAT DEC 0–99 | Ausklang der Hi-Hats (geschlossen kurz, offen länger) |
| KIT | HAT TONE 0–99 | Klangfarbe der Hi-Hats: 0 dunkler, metallischer, 99 hell und dünn |
| KIT | DRIVE 0–99 | Sättigung aller Instrumente: 0 sauber, höher kräftiger und rauer |

Die anderen Instrumente (Rimshot, Clap, Cowbell, Becken, das Instrument auf G#) behalten die Einstellungen ihres
Kits. Die Toms setzen etwas höher ein und fallen dann auf ihren Ton, wie bei 808 und 909. Im Browser-Emulator
kommt jede Taste mit gleicher Stärke an, der Accent lässt sich dort nicht spielen.

## ACID

Ein einstimmiger Bass im Stil der Roland TB-303, berechnet auf [Open303](https://github.com/RobinSchmidt/Open303)
von Robin Schmidt (MIT, nach C portiert), mit dessen 303-Filter. Es klingt immer nur ein Ton.

- **Slide:** Eine Taste halten und eine zweite drücken: Der Ton gleitet zur neuen Note, ohne neuen Anschlag. Wird
  die zuletzt gedrückte losgelassen, während die erste noch gehalten ist, gleitet er zu ihr zurück.
- **Accent:** ENV halten und dann die Taste anschlagen: Die Note ist akzentuiert, lauter und mit kräftigerem
  Filter-Schwung. Solange ENV gehalten ist, steht `ACC` in der Kopfzeile. Über MIDI ist eine Note mit Velocity ab
  100 akzentuiert. Die Anschlagdynamik der Tasten wirkt bei ACID sonst nicht.
- Wie bei der 303 klingt ein gehaltener Ton über etwa eine Sekunde aus. Beim Loslassen ist er sofort weg, eine
  akzentuierte Note nach etwa 50 ms.

| Seite | Parameter | Was er tut |
| --- | --- | --- |
| FILTER | TUNE −12 … +12 | Stimmung in Halbtönen; ein gehaltener Ton gleitet dorthin |
| FILTER | CUTOFF 0–99 | Grenzfrequenz des Filters, 314 Hz bis 2,4 kHz |
| FILTER | RESO 0–99 | Resonanz: höher quäkt und pfeift es, bei 99 fast ein Pfeifton |
| FILTER | ENV MOD 0–99 | wie weit die Hüllkurve das Filter im Anschlag öffnet |
| VOICE | DECAY 0–99 | wie schnell sich das Filter nach dem Anschlag schließt, 0,2 bis 2 s; ab der nächsten Note |
| VOICE | ACCENT 0–99 | wie stark akzentuierte Noten hervortreten; ab der nächsten Note |
| VOICE | WAVE 0–99 | Wellenform: 0 Sägezahn, 99 Rechteck, dazwischen gemischt |
| VOICE | SLIDE 0–99 | Gleitzeit beim Slide, in ms (Zeitkonstante); 60 ist die der 303, 0 springt ohne Gleiten |

Presets: ACID LINE, SQUELCH, DEEP SAW, SQUARE, RUBBER, SCREAM, PLUCK, SLOW SLIDE. Für Bass die Oktave mit OCT−
nach unten stellen. In hohen Lagen können SCREAM und SQUELCH mit Accent bei vollem MASTER übersteuern.
