# KLASSIK – Bedienung (v0.3, Teil DRUMS)

## Rack

| Bedienelement | Was es tut |
| --- | --- |
| Tasten | spielen die gewählte Engine (Anschlagdynamik), senden die Noten über USB-MIDI |
| K1 | Engine wählen, in der Reihenfolge des Racks: FM6, VA (ANALOG), PD (PHASE), ORGAN (WHEEL), DRUMS |
| PRESET | Sound der Engine wählen; nach den Presets folgt USER, sobald die Engine einen eigenen Sound hat |
| OCT- / OCT+ | Oktave −2 … +2 |
| EDIT | Edit-Seiten der Engine öffnen |
| SAVE | Engine, Sounds und Oktave speichern (leuchtet, solange etwas ungespeichert ist) |
| MASTER | Lautstärke |

MIDI-In über USB und TRS spielt ebenfalls. ACID ist als Kachel sichtbar, aber noch nicht spielbar.

## Edit-Seiten

Jede Engine hat zwei Seiten mit je vier Parametern: FM6 ALGO und ENV, VA OSC und FLT, PD PHS und LINE, ORGAN BARS
und TONE, DRUMS DRUM und KIT. In der Mitte steht die Glyphe des Sounds groß; die Namen an den Strahlen der vier
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
| Instrument | BD | RS | SD | CP | CB | LT | CH | MT | CH | HT | OH | CY |

BD Bassdrum, RS Rimshot, SD Snare, CP Clap, CB Cowbell, LT / MT / HT tiefe, mittlere, hohe Tom, CH geschlossene,
OH offene Hi-Hat, CY Becken. BD, RS, SD, CP, CH und OH liegen dort, wo General MIDI sie hat: Ein Drum-Sequencer per
MIDI trifft sie.

- Der Anschlag ist der Accent: härter gespielt klingt lauter und kräftiger.
- Jedes Instrument hat eine Stimme, wie bei den Originalen: nochmal angeschlagen, klingt es von dort weiter, wo es
  gerade ist.
- Die geschlossene Hi-Hat beendet eine klingende offene.
- Das Loslassen einer Taste ändert nichts: Jeder Schlag klingt von selbst aus.

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
| KIT | HAT TONE 0–99 | Klangfarbe der Hi-Hats: 0 dunkel, 99 hell |
| KIT | DRIVE 0–99 | Sättigung aller Instrumente: 0 sauber, höher kräftiger und rauer |

Die anderen Instrumente (Rimshot, Clap, Cowbell, Becken) behalten die Einstellungen ihres Kits. Im Browser-Emulator
kommt jede Taste mit gleicher Stärke an, der Accent lässt sich dort nicht spielen.
