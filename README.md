# KLASSIK – klassische Synth-Klänge für den M-VAVE FM-1

Eine eigene Firmware für den M-VAVE FM-1: mehrere klassische Synth-Engines in einem „Rack“, mit einer
übersichtlichen Oberfläche, in der jeder Sound ein eigenes Symbol hat (seine **Glyphe**: die acht Parameter des
Sounds als acht Strahlen).

**Status: v0.1, experimentell.** Getestet nur im Host-Simulator, noch nicht auf einem FM-1.

![Rack im Host-Simulator (240×240)](docs/img/rack-va.png)

## Was v0.1 kann

- Drei Engines aus [Felucca-Engines](https://github.com/hugelton/Felucca-Engines) (MIT): **VA** (ANALOG),
  **PD** (PHASE, Phase Distortion), **ORGAN** (WHEEL, Zugriegel-Orgel); sechs Stimmen.
- Rack-Bildschirm mit Glyphen, Kopfzeile, Reglerzeile; FM6, STRINGS und ACID als Kacheln „SOON“.
- Edit-Seiten: EDIT öffnet je Engine zwei Seiten mit je vier Parametern auf K1–K4. Die Glyphe in der Kopfzeile
  und eine große Live-Glyphe verformen sich beim Drehen, der Klang ändert sich sofort.
- Eigener Sound: SAVE speichert einen geänderten Sound als USER-Sound seiner Engine (einer je Engine).
- Tasten mit Anschlagdynamik, MIDI-In (USB, TRS), MIDI-Out (USB), Oktave, Speichern über den Neustart hinaus,
  Screenreader-Ansagen (fm1-reader).

Bedienung: [firmware/klassik/docs/MANUAL.md](firmware/klassik/docs/MANUAL.md). Konzept und Fahrplan:
[docs/CONCEPT.md](docs/CONCEPT.md).

## Aufbau

| Pfad | Was |
| --- | --- |
| `base/` | [fm1-base](https://github.com/charlesvestal/fm1-base) (Git-Submodul, MPL-2.0): Plattform, Host-Simulator, Browser-Emulator, Build-Werkzeuge |
| `engines/` | [Felucca-Engines](https://github.com/hugelton/Felucca-Engines) (Git-Submodul, MIT) |
| `firmware/klassik/app.c` | die App: Bildschirm, Bedienung, Einstellungen, Audio-Ausgabe |
| `firmware/klassik/dsp/engines.c` | der Stimmen-Host für die Engines (MIT, nach `engines/tests/fe_host.c`) |
| `firmware/klassik/fm1.toml` | Build-Einstellungen |
| `firmware/klassik/tests/*.txt` | Tests für den Host-Simulator (mit Screenshots und WAV) |
| `tools/setup.sh` | holt die Submodule in den getesteten Versionen |

## Erste Schritte (Mac)

Voraussetzungen: Git, ein C-Compiler (`xcode-select --install`), [Homebrew](https://brew.sh), Python 3.

1. FriBiDi installieren. Der Build rendert die Schriften mit Pillow (`base/tools/gen_font.py`); Pillow nutzt dafür
   libraqm, und libraqm lädt FriBiDi zur Laufzeit:

       brew install fribidi

2. Eine Python-Umgebung mit Pillow im Repository anlegen (`.venv/` steht in `.gitignore`):

       python3 -m venv .venv
       .venv/bin/pip install Pillow

3. Prüfen, ob Pillow FriBiDi findet. Erwartet: `True` und eine Versionsnummer, z. B. `True 1.0.17`:

       .venv/bin/python -c "from PIL import features; print(features.check('raqm'), features.version('fribidi'))"

   Steht dort `False`, findet macOS die Bibliotheken von Homebrew nicht von selbst. Dann vor dem Build (oder
   dauerhaft in `~/.zshrc`):

       export DYLD_FALLBACK_LIBRARY_PATH="$(brew --prefix)/lib"

4. Submodule holen, den Host-Simulator bauen, alle Tests laufen lassen (jede Zeile „ok“):

       sh tools/setup.sh
       .venv/bin/python base/tools/build.py firmware/klassik --host
       for t in firmware/klassik/tests/*.txt; do firmware/klassik/build/host/klassik_host "$t" firmware/klassik/build/host || break; done

Die Screenshots (u. a. `boot.png`, `edit-va-1.png`, `edit-va-2.png`) und die WAVs (`va.wav`; `edit-cut.wav`, ein
Filter-Sweep beim Drehen) liegen danach in `firmware/klassik/build/host/`.

Im Browser (braucht Emscripten, z. B. `brew install emscripten`):

    .venv/bin/python base/tools/build.py firmware/klassik --web      # firmware/klassik/build/emu/

Für den FM-1 selbst (braucht Docker Desktop und die JieLi-Toolchain, siehe `base/README.md`):

    .venv/bin/python base/tools/build.py firmware/klassik            # firmware/klassik/build/klassik.fwsc

Der Geräte-Build ist noch **nicht** getestet. Vor dem ersten Flashen: Backup, und die Community-Regel beachten,
vor einem Wechsel zwischen Custom-Firmwares erst auf die Stock-Firmware V15 zurückzugehen.

## Lizenz

Eigene Dateien: MPL-2.0 (`LICENSE`), Details zu allen Teilen in [LICENSING.md](LICENSING.md).
