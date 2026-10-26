# Entscheidungen

Neueste unten. Format: Datum · Entscheidung · Begründung · Status.

| Datum | Entscheidung | Begründung | Status |
| --- | --- | --- | --- |
| 25.10. | Ziel: eigene FM-1-Firmware mit mehreren klassischen Synth-Engines, eigenständige, übersichtliche UI | Fabios Vorgabe | fest |
| 25.10. | Basis **fm1-base** statt Felucca-Fork oder fm1-dock | MPL-2.0 (freie Lizenzwahl), Simulator, sauberer Start für eigene UI | von Claude empfohlen, mit dem Repo-Gerüst übernommen |
| 25.10. | Keine GPL-Quellen; Engines aus Felucca-Engines (MIT), später msfa (Apache-2.0) und Open303 (MIT) | Lizenz bleibt frei wählbar | fest |
| 26.10. | Eigene Dateien unter **MPL-2.0** | gleiche Lizenz wie fm1-base | Vorschlag von Claude, von Fabio noch nicht ausdrücklich bestätigt |
| 26.10. | Display **240×240** | mehrere Quellen, siehe RESEARCH.md; erste Mockups (320×240) verworfen | fest |
| 26.10. | **Glyphe** als durchgehendes Erkennungszeichen jedes Sounds; Raster Kopfzeile / Inhalt / Reglerzeile | Signatur der UI | fest |
| 26.10. | Signatur-Ideen Klang-Atlas, Glyphen-Browser, Klang-Stammbaum | in Verzeichnissen nicht gefunden (Neuheit unsicher) | geplant (v0.4–v0.6) |
| 26.10. | v0: VA (ANALOG), PD (PHASE), ORGAN (WHEEL); 1 Part, 6 Stimmen | schnell spielbar, MIT | umgesetzt, im Simulator getestet |
| 26.10. | Sequencer und Mixer mit 4 Parts | Annahme für die Mockups (Stock V16 hat 8) | offen |
| 26.10. | Repo `fabiomorena/fm1-klassik`, CI mit Host-Tests | Sicherung und automatische Prüfung | umgesetzt, CI grün |
| 26.10. | Arbeit nur noch in PyCharm mit Claude Code; `docs/design/` ersetzt das Canvas | Fabios Wunsch | fest |
| 26.10. | v0.1: EDIT mit zwei Seiten à vier Parametern auf K1–K4, Live-Glyphe, ein USER-Sound je Engine (SAVE); Hüllkurve, Pegel und Regler-Beschleunigung erst mit Gerät | acht Parameter je Engine (`engine_t.edit[8]`, `page_title[2]`) passen auf K1–K4; Feinabstimmung braucht Hardware | umgesetzt, im Simulator getestet (Commit `aa1433c`) |

## Offene Punkte

- `id_prefix = "FM-1_77"` vor dem ersten Release auf Kollisionen prüfen.
- Ausgangspegel (×32) auf Hardware einmessen.
- Geräte-Build (Docker, JieLi-Toolchain) einrichten und im fm1-emu testen.
- Beim Kauf des Geräts: ausgelieferte Firmware-Version prüfen (Werkzeuge sind auf V15 ausgelegt).
- Lizenz MPL-2.0 bestätigen oder ändern.
