# KLASSIK – Bedienung (v0.1)

## Rack

| Bedienelement | Was es tut |
| --- | --- |
| Tasten | spielen die gewählte Engine (Anschlagdynamik), senden die Noten über USB-MIDI |
| K1 | Engine wählen: VA (ANALOG), PD (PHASE), ORGAN (WHEEL) |
| PRESET | Sound der Engine wählen; nach den Presets folgt USER, sobald die Engine einen eigenen Sound hat |
| OCT- / OCT+ | Oktave −2 … +2 |
| EDIT | Edit-Seiten der Engine öffnen |
| SAVE | Engine, Sounds und Oktave speichern (leuchtet, solange etwas ungespeichert ist) |
| MASTER | Lautstärke |

MIDI-In über USB und TRS spielt ebenfalls. FM6, STRINGS und ACID sind als Kacheln sichtbar, aber noch nicht spielbar.

## Edit-Seiten

Jede Engine hat zwei Seiten mit je vier Parametern: VA OSC und FLT, PD PHS und LINE, ORGAN BARS und TONE. In der
Mitte steht die Glyphe des Sounds groß; die Namen an den Strahlen der vier Parameter auf K1–K4 leuchten in der
Farbe der Engine.

| Bedienelement | Was es tut |
| --- | --- |
| K1–K4 | die vier Parameter der Seite; Klang und Glyphen ändern sich sofort |
| EDIT oder OCT+ | die andere Seite |
| OCT- | zurück zum Rack |
| PRESET | anderen Sound laden (eine ungespeicherte Änderung geht dabei verloren) |
| SAVE | wie im Rack, siehe unten |

Ein geänderter, noch nicht gespeicherter Sound hat ein `*` hinter dem Namen, und SAVE leuchtet. SAVE macht ihn zum
USER-Sound seiner Engine und speichert alles über den Neustart hinaus. Jede Engine hat einen USER-Sound; ein neuer
ersetzt den alten. Er behält Hüllkurve und MONO des Presets, aus dem er entstanden ist. Wer zwischendurch die Engine
wechselt, behält die Änderungen bis zum Ausschalten; ein SAVE speichert die geänderten Sounds aller Engines.
