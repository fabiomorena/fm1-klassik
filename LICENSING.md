# Lizenzen

| Teil | Lizenz | Quelle |
| --- | --- | --- |
| KLASSIKs eigene Dateien (`firmware/klassik/app.c`, `app.h`, `dsp/klassik_dsp.h`, Werkzeuge, Doku) | MPL-2.0 | dieses Repository |
| `firmware/klassik/dsp/engines.c` | MIT | enthält den Stimmen-Start und die Render-Schleife aus Felucca-Engines' `tests/fe_host.c` |
| `base/` (fm1-base, darin felucca-bsp) | MPL-2.0 | https://github.com/charlesvestal/fm1-base |
| `engines/` (Felucca-Engines: ANALOG, PHASE, WHEEL u. a.) | MIT | https://github.com/hugelton/Felucca-Engines |
| JieLi AC79 SDK (drei Dateien im `.fwsc`-Paket, beim Geräte-Build geholt) | Apache-2.0 | siehe `base/LICENSING.md` |

Ziel: keine GPL-Teile, damit die Lizenz der Firmware frei wählbar bleibt. Geplant sind deshalb für FM6 der
msfa-Kern (Apache-2.0, nicht Dexed selbst, das GPL-3.0 ist) und für ACID Open303 (MIT). Jede neue Quelle wird
vor der Übernahme hier eingetragen.
