# KLASSIK – Bedienung (v0.2)

## Rack

| Bedienelement | Was es tut |
| --- | --- |
| Tasten | spielen die gewählte Engine (Anschlagdynamik), senden die Noten über USB-MIDI |
| K1 | Engine wählen, in der Reihenfolge des Racks: FM6, VA (ANALOG), PD (PHASE), ORGAN (WHEEL) |
| PRESET | Sound der Engine wählen; nach den Presets folgt USER, sobald die Engine einen eigenen Sound hat |
| OCT- / OCT+ | Oktave −2 … +2 |
| EDIT | Edit-Seiten der Engine öffnen |
| SAVE | Engine, Sounds und Oktave speichern (leuchtet, solange etwas ungespeichert ist) |
| MASTER | Lautstärke |

MIDI-In über USB und TRS spielt ebenfalls. STRINGS und ACID sind als Kacheln sichtbar, aber noch nicht spielbar.

## Edit-Seiten

Jede Engine hat zwei Seiten mit je vier Parametern: FM6 ALGO und ENV, VA OSC und FLT, PD PHS und LINE, ORGAN BARS
und TONE. In der Mitte steht die Glyphe des Sounds groß; die Namen an den Strahlen der vier Parameter auf K1–K4
leuchten in der Farbe der Engine.

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
