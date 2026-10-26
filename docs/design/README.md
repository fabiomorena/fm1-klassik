# Mockups

Alle Screens des UI-Konzepts, gezeichnet in 2× (480×480 = 240×240 auf dem Gerät). Seit dem 26.10.2026 ist **dieser
Ordner die maßgebliche Quelle** für das Design; das frühere Design-Canvas in claude.ai wird nicht mehr gepflegt.

**Ansehen:** `index.html` im Browser öffnen (in PyCharm: Rechtsklick auf die Datei → im Browser öffnen). Die Seite
zeigt alle zehn Screens nebeneinander, jeweils direkt aus der HTML-Datei.

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

`png/`: Renderings vom 26.10.2026, ohne die Webfonts des Canvas erstellt (daher breitere Ersatzschrift und vereinzelt
Umbrüche). Nach Änderungen bei Bedarf neu erzeugen, z. B. mit Chrome im Headless-Modus (nicht getestet):

    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome" --headless --window-size=480,480 \
        --screenshot=docs/design/png/Main.png "file://$PWD/docs/design/Main.dc.html"

Werte, Soundnamen und Glyphen in den Mockups sind Beispiele; maßgeblich für die Glyphe ist `glyph_draw` in
`firmware/klassik/app.c`.
