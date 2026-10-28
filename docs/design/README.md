# Mockups

Alle Screens des UI-Konzepts, gezeichnet in 2× (480×480 = 240×240 auf dem Gerät). Seit dem 26.10.2026 ist **dieser
Ordner die maßgebliche Quelle** für das Design; das frühere Design-Canvas in claude.ai wird nicht mehr gepflegt.

**Ansehen:** `index.html` im Browser öffnen (in PyCharm: Rechtsklick auf die Datei → im Browser öffnen). Die Seite
zeigt alle Screens nebeneinander, jeweils direkt aus der HTML-Datei.

**Ändern:** direkt in der jeweiligen `*.dc.html`. Maße und Farben stehen in den Inline-Styles, alle Werte sind 2× der
Geräte-Pixel. `<x-dc>`, `<helmet>`, `support.js` und das Skript am Dateiende stammen aus dem Canvas und haben im
Browser keine Wirkung.

| Datei | Screen |
| --- | --- |
| `Main` | 01 Rack: Engine-Wahl |
| `Edit-VA` | 02 VA: Filterseite mit Live-Glyphe |
| `Edit-FM6` | 03 FM6: Algorithmus und Operator-Pegel |
| `Presets` | 04 Preset-Liste mit Glyphen |
| `Sequencer` | 05 Sequencer: 4 Parts × 16 Steps |
| `Mixer` | 06 Mixer: 4 Parts + Master |
| `FX` | 07 FX-Kette mit Delay-Detail |
| `Atlas` | 08 Klang-Atlas |
| `Glyphs` | 09 Glyphen-Browser |
| `Family` | 10 Klang-Stammbaum |
| `Edit-DRUMS` | 11 DRUMS: eine Oktave als Pads (jede Taste mit ihrem Instrument), Seiten DRUM und KIT |
| `Edit-ACID` | 12 ACID: Seiten FILTER (TUNE, CUTOFF, RESO, ENV MOD) und VOICE (DECAY, ACCENT, WAVE, SLIDE), Glyphe wie bei VA; `ACC` in der Kopfzeile, solange der Accent-Taster (ENV) gehalten wird |
| `Edit-Reset` | 13 Regler zurücksetzen, auf jeder Edit-Seite: HOME halten und K1–K4 drehen, dann springt der Regler auf den Wert seines Sounds (Preset oder USER). HOME zeigt kurz `RESET TURN K1-K4`; eine Marke im Balken zeigt den Wert des Sounds, solange der Regler woanders steht |
| `Rack-Parts` | 14 Rack mit vier Parts: Kopfzeile „1 ACID“ (Part und Engine), auf jeder Kachel die Parts, die ihre Engine spielen (der gewählte gefüllt, die anderen umrandet); SEL (Taster) oder SELECT (Drehregler) wechselt den Part |
| `Mixer-Parts` | 15 Mixer von v0.5 (GLO öffnet, OCT− zurück): je Part ein Zug mit LEVEL (−40 … +6 dB, darunter aus), PAN, MUTE, SOLO auf K1–K4 für den gewählten Part; MASTER mit Pegel L/R und `LIM`; HOME + K1–K4 zurück auf 0 dB, Mitte, aus, mit Marke bei LEVEL und PAN. Ziel mit Sends und Seiten bleibt `Mixer` (v0.6) |
| `Looper` | 16 Noten-Looper von v0.5, Tab LOOP (SEQ öffnet; SEQ oder OCT+ zu STEP, OCT− zurück): je Part eine Spur mit seinen Noten, eine Schleife für alle; PLAY startet und stoppt, REC nimmt auf den gewählten Part auf (Overdub), die erste Aufnahme legt die Länge fest; HOME + REC nimmt die letzte Aufnahme zurück (gleich welcher Part), HOME + PLAY leert den Part; K1 TEMPO, K2 LENGTH, K3 QUANT, K4 CLICK, HOME + K1–K4 auf 120 BPM, Länge der ersten Aufnahme, 1/16, REC |
| `Looper-Rack` | 17 Der Looper auf den anderen Screens: rechts in der Kopfzeile ein roter Punkt bei Aufnahme (grünes Dreieck beim Spielen), dazu Takt und Schlag; hier die erste Aufnahme im Rack |
| `Step` | 18 Step-Sequencer von v0.6: Tab STEP der Looper-Seite (SEQ oder OCT+ wechselt LOOP/STEP); ein Takt (K1 BAR) als 16 Steps je Part, dieselben Noten wie im Looper; die 16 weißen Tasten sind die Steps des gewählten Parts (Licht = gesetzt, Lauflicht = spielt); Taste halten + K2 NOTE, K3 VEL, K4 LEN ändert den Step; hier ACID, Step 9 gehalten |
| `Step-DRUMS` | 19 Step-Sequencer mit DRUMS: K2 wählt das Instrument (INST), die Tasten zeigen und setzen nur dessen Steps wie bei einer TR; darunter seine Reihe und die Instrumente des Kits (Punkt = hat Steps) |

`png/`: Renderings vom 26.10.2026, ohne die Webfonts des Canvas erstellt (daher breitere Ersatzschrift und vereinzelt
Umbrüche); `Main.png`, `Edit-DRUMS.png`, `Edit-ACID.png`, `Edit-Reset.png`, `Rack-Parts.png`, `Mixer-Parts.png`, `Looper.png`, `Looper-Rack.png` (27.10.2026), `Step.png`, `Step-DRUMS.png` und `Looper.png` (Tab-Zeile, 28.10.2026) neu mit Chrome im Headless-Modus (26.10.2026, so erprobt):

    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome" --headless --disable-gpu --hide-scrollbars \
        --user-data-dir=/tmp/chrome-mockups --window-size=480,480 \
        --screenshot=docs/design/png/Main.png "file://$PWD/docs/design/Main.dc.html"

Werte, Soundnamen und Glyphen in den Mockups sind Beispiele; maßgeblich für die Glyphe ist `glyph_draw` in
`firmware/klassik/app.c`.
