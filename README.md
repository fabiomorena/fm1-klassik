# KLASSIK – klassische Synth-Klänge für den M-VAVE FM-1

Eine eigene Firmware für den M-VAVE FM-1: mehrere klassische Synth-Engines in einem „Rack“, mit einer
übersichtlichen Oberfläche, in der jeder Sound ein eigenes Symbol hat (seine **Glyphe**: die acht Parameter des
Sounds als acht Strahlen).

**Status: v0, experimentell.** Getestet nur im Host-Simulator, noch nicht auf einem FM-1.

![Rack im Host-Simulator (240×240)](docs/img/rack-va.png)

## Was v0 kann

- Drei Engines aus [Felucca-Engines](https://github.com/hugelton/Felucca-Engines) (MIT): **VA** (ANALOG),
  **PD** (PHASE, Phase Distortion), **ORGAN** (WHEEL, Zugriegel-Orgel); sechs Stimmen.
- Rack-Bildschirm mit Glyphen, Kopfzeile, Reglerzeile; FM6, STRINGS und ACID als Kacheln „SOON“.
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
| `firmware/klassik/tests/basic.txt` | Tests für den Host-Simulator (mit Screenshots und WAV) |
| `tools/setup.sh` | holt die Submodule in den getesteten Versionen |

## Erste Schritte (Mac)

Voraussetzungen: Git, Python 3 mit Pillow (`pip3 install Pillow`), ein C-Compiler (`xcode-select --install`).

    sh tools/setup.sh
    python3 base/tools/build.py firmware/klassik --host
    firmware/klassik/build/host/klassik_host firmware/klassik/tests/basic.txt firmware/klassik/build/host

Die Screenshots (`boot.png`, `phase.png`, `organ.png`) und `va.wav` liegen danach in
`firmware/klassik/build/host/`.

Im Browser (braucht Emscripten, z. B. `brew install emscripten`):

    python3 base/tools/build.py firmware/klassik --web      # firmware/klassik/build/emu/

Für den FM-1 selbst (braucht Docker Desktop und die JieLi-Toolchain, siehe `base/README.md`):

    python3 base/tools/build.py firmware/klassik            # firmware/klassik/build/klassik.fwsc

Der Geräte-Build ist noch **nicht** getestet. Vor dem ersten Flashen: Backup, und die Community-Regel beachten,
vor einem Wechsel zwischen Custom-Firmwares erst auf die Stock-Firmware V15 zurückzugehen.

## Lizenz

Eigene Dateien: MPL-2.0 (`LICENSE`), Details zu allen Teilen in [LICENSING.md](LICENSING.md).
