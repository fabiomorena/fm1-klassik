# EDIT (v0.8) – Testprotokoll (Browser-Emulator)

Stand v0.8: Hüllkurve (Tabs ENV und MORE, Taster ENV), VA-Filterkurve, FM6-Algorithmus. Die erwarteten Ergebnisse
stammen aus dem Code (`firmware/klassik/app.c`, `firmware/klassik/dsp/engines.c`), den Host-Tests
(`firmware/klassik/tests/env.txt`, `edit.txt`, `fm6.txt`) und Messungen im Simulator (RESEARCH.md „Hüllkurve“,
„Filter von ANALOG“, „FM6: Algorithmen als Bild“). Es geht um Funktion und Stabilität; wie es klingt und aussieht,
stimmen wir zum Schluss ab (Notizen dazu sind trotzdem willkommen).

| Getestet am | Browser | Dauer |
| --- | --- | --- |
| | | |

## So markierst du

- `[x]` wie erwartet
- `[!]` Abweichung, darunter eine Zeile `  - Notiz: …` (was du gemacht hast, was passiert ist)
- `[-]` nicht getestet

Wenn du fertig bist: Datei speichern und mir „Protokoll fertig“ schreiben.

## 0 Vorbereitung

| Was | Wie im Emulator |
| --- | --- |
| Regler drehen | Mausrad über dem Regler, oder Regler anklicken und Pfeiltasten (Bild↑ / Bild↓: 5 Schritte) |
| Taste oder Taster festhalten | Rechtsklick oder Shift-Klick: bleibt gedrückt, bis du noch einmal klickst |
| weiße Tasten | `Q W E R T Z U I` = F3 … F4, `A S D F G H J K` = G4 … G5 (`T` = C4) |
| HOME | `Esc` (festhalten und einen Regler drehen: zurück auf den Wert des Sounds) |
| OCT− / OCT+ | `Y` / `X` |
| SEL, SELECT | SEL anklicken; SELECT: Pfeiltasten ↑ / ↓ |

Die Tastatur-Kürzel wirken nur, wenn das Panel den Fokus hat: einmal aufs Gerät klicken.

**Achtung Pfeiltasten:** Hast du zuletzt einen Regler angeklickt, drehen ↑ / ↓ diesen Regler statt SELECT. Für
SELECT den Drehregler SELECT anklicken (das dreht ihn nicht), dann ↑ / ↓.

- [ x] 0.1 (Wenn du willst: das neue Speicherformat) `http://localhost:8000` mit **Cmd+Shift+R** neu laden, **ohne**
  Factory reset „Switch on“: Parts, Sounds, LFO- und ARP-Einstellungen wie zuletzt gespeichert (v0.7 wird
  übernommen; ein USER-Sound bekommt die Hüllkurve seines Presets).
- [ x] 0.2 „Factory reset“ zweimal klicken (der zweite Klick bestätigt), dann „Switch on“. Neuer Stand? Der Taster ENV
  öffnet bei Part 1 (VA) eine Seite (statt `ENV SOON`).

## A Hüllkurve: ENV und MORE (Part 1 VA, SAW LEAD)

- [ x] A1 ENV: die Edit-Seite auf dem Tab `ENV`, Tabs `OSC FLT ENV MORE`, rechts `3/4`; ENV leuchtet. Oben
  `ENV - AMP + FILTER` und `SUS 79%`, die Kurve (steil hoch, gebogen ab, gerade, gebogen aus), darunter `A D S R`;
  unten `ENV > FILTER` mit `+20` und einem Balken, rechts die Glyphe (`LIVE`). Fußzeile `K1 ATK 2 MS`,
  `K2 DEC 205 MS`, `K3 SUS 79%`, `K4 REL 172 MS`.
- [ x] A2 Eine Taste halten: ein weißer Punkt läuft die Kurve hoch und bleibt auf der Geraden (SUS); loslassen: er
  läuft das Ausklingen hinunter und verschwindet.
- [ x] A3 K1 ATK hoch (bis `1.5 S`): der Ton schwillt langsam an. K3 SUS auf `0%`, K2 DEC: kurz (Zupfen) bis lang
  (bis `2.5 S`). K4 REL: kurz bis lang (bis `4.9 S` Nachklingen). Jeder Reglerschritt ändert die Zeit (keine Stufen),
  das Bild folgt.
- [x ] A4 Der Sound heißt jetzt `SAW LEAD*`, SAVE leuchtet, die Balken haben Marken. HOME halten und K1–K4 drehen:
  zurück auf 2 MS, 205 MS, 79 %, 172 MS; `*` und Marken weg.
- [x ] A5 ENV noch einmal: Tab `MORE` (`4/4`). Oben `ENV > FILTER +20`: gestrichelt der Cutoff (`CUT 71%`), darüber
  die Kurve. K1 ENV>FLT hoch (bis `+63`): jeder Anschlag öffnet den Filter hörbar („Wah“), das Bild hebt sich; negativ
  (bis `-64`): der Anschlag wird dumpfer, das Bild senkt sich. K3 und K4 zeigen `-` und tun nichts.
- [x ] A6 K2 VOICE: SAW LEAD ist `MONO` (eine Stimme, `ONE VOICE, LEGATO`): zwei Tasten nacheinander halten, der Ton
  gleitet ohne neuen Anschlag. Auf `POLY` (`SIX VOICES`): Akkorde klingen. Während Tasten gehalten sind umschalten:
  was klingt, klingt aus; danach hängt kein Ton.
- [x ] A7 ENV auf MORE: zurück zu ENV. EDIT oder OCT+: weiter durch `OSC`, `FLT`, `ENV`, `MORE`, wieder `OSC`. OCT−
  zurück zum Rack.

Notizen: das switchen von env und more mit klicken auf env sollte auch durch alle tabs scrollen können, nicht
nur bei oct + - gerade bemerkt mit edit scrollt man auch durch alle tabs

## B Andere Engines

- [ x] B1 SEL zu Part 5 (PD): ENV zeigt `ENV - AMP + DCW`; MORE heißt K1 `ENV>DCW` (die Tiefe der Phasenverzerrung
  folgt der Hüllkurve; Bild: `DCW` gestrichelt).
- [x ] B2 Part 6 (ORGAN): ENV `ENV - AMP`; die Hüllkurve wirkt (ATK hoch: die Orgel schwillt an, REL lang: sie klingt
  nach). MORE: oben `ORGAN: NO FILTER`, K1 `-` tut nichts; VOICE geht.
- [ x] B3 Part 4 (FM6): ENV öffnet seine Seite `ENV` (Tabs `ALGO ENV`, `2/2`) mit `ENV - CARRIERS` und der Kurve aus
  ATK, DEC, SUS, REL (Zahlen 0–99, keine Zeiten, kein Punkt). ENV noch einmal: bleibt dort.
- [x ] B4 Part 3 (ACID): ENV öffnet keine Seite; gehalten beim Anschlag ist es weiter der Accent (`ACC` in der
  Kopfzeile).
- [x ] B5 Part 2 (DRUMS): ENV zeigt kurz `NO ENV`, keine Seite.

## C VA: Filterkurve (Tab FLT)

- [x ] C1 Part 1 VA, EDIT, EDIT (Tab `FLT`, `2/4`): oben `LOW PASS 12 DB` und rechts die Frequenz (`2.6 KHZ`), die
  Kurve von `20` bis `20K HZ`, gestrichelt der Cutoff; unten die Hüllkurve klein mit `ENV > FILTER +20`, `DRV`, `KTR`,
  die Glyphe.
- [ x] C2 K1 CUT drehen: die Linie und die Frequenz wandern (`30 HZ` ganz links bis `16.0 KHZ`), der Klang wird
  dunkler und heller. K2 RES hoch: eine Spitze am Cutoff, der Klang pfeift.

## D FM6: Algorithmus (Tab ALGO)

- [ x] D1 Part 4 FM6 (E.PIANO), EDIT: oben `ALGORITHM` und `ALG 05`: drei Paare, `2` über `1`, `4` über `3`, `6` über
  `5`, die unteren gefüllt (Träger), eine Schleife an `6` (Feedback), unten `OUT`. Darunter `OP1` … `OP6` mit `C`
  oder `M` und ihren Pegeln.
- [ x] D2 K1 ALGO von 1 bis 32 drehen: das Bild ändert sich mit jedem Algorithmus (1: `6 5 4` über `3` und `2` über
  `1`; 32: sechs Träger nebeneinander), die Kästen bleiben auf dem Bildschirm.
- [ x] D3 K3 MOD drehen: die Balken der `M`-Operatoren wandern, die der `C`-Operatoren bleiben; der Klang wird mit
  weniger MOD weicher.

## E Speichern und Stabilität

- [x ] E1 VA: auf ENV und MORE etwas ändern (z. B. DEC, REL, ENV>FLT, VOICE POLY), SAVE: `SAVED USER`. Seite neu laden
  und „Switch on“: VA spielt `USER` mit genau diesen Werten.
- [ x] E2 Fünf Minuten spielen, dabei auf ENV, MORE, FLT und ALGO drehen, Parts und Seiten wechseln, mit Looper:
  keine Aussetzer, kein Absturz, kein hängender Ton.

## Auswertung (Claude, 29.10.2026)

- Alle 23 Punkte ohne Befund (`[x ]`, `[ x]` als bestanden gewertet): die Übernahme des v0.7-Stands ohne Factory reset
  (0.1), ENV und MORE bei VA (Bild, Punkt der laufenden Note, Zeiten ohne Stufen, HOME-Reset, ENV > FILTER, VOICE mit
  Umschalten ohne hängenden Ton, Tabs), PD (ENV > DCW), ORGAN (kein Filter), FM6 (seine ENV-Seite), ACID (Accent),
  DRUMS (`NO ENV`), die Filterkurve von VA, der FM6-Algorithmus mit allen 32 und den Operator-Pegeln, Speichern, fünf
  Minuten Stabilität.
- Notiz unter A7: „das switchen von env und more mit klicken auf env sollte auch durch alle tabs scrollen können“.
  Nachgefragt (ENV wie EDIT, oder dazu OCT− einen Tab zurück, oder so lassen): **ENV wie EDIT** gewählt. Umgesetzt:
  bei geschlossener Seite öffnet ENV auf ENV, auf einer offenen Edit-Seite geht ENV einen Tab weiter wie EDIT und
  OCT+ (VA: ENV → MORE → OSC → FLT → ENV; FM6: ENV → ALGO → ENV); OCT− schließt weiter. Host-Test `tests/env.txt`.
- Dabei gefunden und behoben (vom Host-Test, nicht im Protokoll): beim Umlauf von MORE auf OSC sagte die App
  `EDIT ENV` statt `EDIT OSC` (der Titel kam aus der Seitennummer vor dem Umlauf); betraf auch EDIT und OCT+ seit
  v0.8, nur die Ansage für den Screenreader, nicht das Bild.

### Nachtest

- [ x] N1 Part 1 VA, ENV (Seite zu): Tab `ENV`. ENV noch einmal: `MORE`, dann `OSC`, `FLT`, wieder `ENV`. OCT−: zurück
  zum Rack.
- [ x] N2 Part 4 FM6: ENV öffnet `ENV`, ENV noch einmal: `ALGO`, noch einmal: `ENV`.

**Nachtest ausgewertet (Claude, 29.10.2026):** N1 und N2 bestanden: ENV geht bei VA und FM6 durch alle Tabs wie EDIT,
OCT− schließt. Damit ist das EDIT-Protokoll fertig.
