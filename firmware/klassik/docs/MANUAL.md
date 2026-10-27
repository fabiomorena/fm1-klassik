# KLASSIK – Bedienung (v0.3)

## Rack

| Bedienelement | Was es tut |
| --- | --- |
| Tasten | spielen die gewählte Engine (Anschlagdynamik; ACID: Accent mit ENV), senden die Noten über USB-MIDI |
| K1 | Engine wählen, in der Reihenfolge des Racks: FM6, VA (ANALOG), PD (PHASE), ORGAN (WHEEL), DRUMS, ACID |
| PRESET | Sound der Engine wählen; nach den Presets folgt USER, sobald die Engine einen eigenen Sound hat |
| OCT- / OCT+ | Oktave −2 … +2 |
| EDIT | Edit-Seiten der Engine öffnen |
| ENV | bei ACID: gehalten, sind die angeschlagenen Noten akzentuiert (`ACC` in der Kopfzeile) |
| SAVE | Engine, Sounds und Oktave speichern (leuchtet, solange etwas ungespeichert ist) |
| MASTER | Lautstärke |

MIDI-In über USB und TRS spielt ebenfalls.

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
| PRESET | anderen Sound laden (eine ungespeicherte Änderung geht dabei verloren) |
| SAVE | wie im Rack, siehe unten |

Ein geänderter, noch nicht gespeicherter Sound hat ein `*` hinter dem Namen, und SAVE leuchtet. SAVE macht ihn zum
USER-Sound seiner Engine und speichert alles über den Neustart hinaus. Jede Engine hat einen USER-Sound; ein neuer
ersetzt den alten. Er behält Hüllkurve und MONO des Presets, aus dem er entstanden ist. Wer zwischendurch die Engine
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
