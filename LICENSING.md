# Lizenzen

| Teil | Lizenz | Quelle |
| --- | --- | --- |
| KLASSIKs eigene Dateien (`firmware/klassik/app.c`, `app.h`, `dsp/klassik_dsp.h`, `dsp/eng_fm6.c` mit den FM6-Presets, `dsp/eng_drums.c` mit den DRUMS-Kits, Werkzeuge, Doku) | MPL-2.0 | dieses Repository |
| `firmware/klassik/dsp/engines.c` | MIT | enthält den Stimmen-Start und die Render-Schleife aus Felucca-Engines' `tests/fe_host.c` |
| `firmware/klassik/dsp/fm6/msfa.c`, `msfa.h`, `firmware/klassik/tools/gen_fm6_tables.py` | Apache-2.0 (Text: `dsp/fm6/LICENSE`) | msfa, der FM-Kern von Googles „Music Synthesizer for Android“ (Copyright 2012–2013 Google Inc.), https://github.com/google/music-synthesizer-for-android, Ordner `app/src/main/jni/` (Stand `master`, Repo archiviert; keine `NOTICE`-Datei). Von C++ nach C portiert und geändert (in den Dateiköpfen beschrieben), wie Apache-2.0 §4(b) es verlangt. Nicht übernommen: Dexed (GPL-3.0) |
| `firmware/klassik/dsp/drums/drums.c`, `drums.h` | MIT (Text in beiden Dateiköpfen) | Plaits-Drums von Mutable Instruments (Copyright 2014, 2016 Emilie Gillet), https://github.com/pichenettes/eurorack (Stand `f4cfa76`), `plaits/dsp/drums/`, dazu Teile von stmlib (`dsp/filter.h`, `dsp/dsp.h`, `dsp/units.h`, `utils/random.h`; https://github.com/pichenettes/stmlib, Stand `d18def8`); MIT laut Dateiköpfen und README („Code (STM32F projects): MIT license“). Nach C portiert, Änderungen im Kopf von `drums.c` |
| ACID (geplant, v0.3) | MIT | Open303 (Copyright (c) 2009 Robin Schmidt), https://github.com/RobinSchmidt/Open303, `License.txt`; aus dem Original-Repo, nicht aus einem GPL-Fork. Nach C portiert |
| `base/` (fm1-base, darin felucca-bsp) | MPL-2.0 | https://github.com/charlesvestal/fm1-base |
| `engines/` (Felucca-Engines: ANALOG, PHASE, WHEEL u. a.) | MIT | https://github.com/hugelton/Felucca-Engines |
| JieLi AC79 SDK (drei Dateien im `.fwsc`-Paket, beim Geräte-Build geholt) | Apache-2.0 | siehe `base/LICENSING.md` |

Ziel: keine GPL-Teile, damit die Lizenz der Firmware frei wählbar bleibt. Deshalb nutzt FM6 den msfa-Kern
(Apache-2.0) und nicht Dexed selbst (GPL-3.0); für ACID ist Open303 (MIT) geplant, für DRUMS Plaits (MIT). Die
FM6-Presets sind eigene Klänge, keine Werksklänge des DX7; die DRUMS-Kits sind synthetisch, ohne Samples aus Rolands
Geräten. Jede neue Quelle wird vor der Übernahme hier eingetragen.
