#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
# Copyright (C) 2026 fabiomorena
"""KLASSIK's interactive manual, made from the repository and the host simulator:

    docs/MANUAL.md       the chapters (see below)
    docs/GLOSSARY.md     the glossary: ## a group, ### an entry (see the file's head)
    docs/scenes/*.txt    the scenes: host scripts (base/host/plat_host.c) told in steps
    docs/manual/         the page's own files (manual.css, manual.js) and the pictures the text names

into build/manual/ (index.html, img/, snd/, fonts/). From the repository's root, after the host build:

    python3 base/tools/build.py firmware/klassik --host
    python3 firmware/klassik/tools/gen_manual.py [--strict]

The page has a space for each chapter, one for the overview and one for the glossary. In MANUAL.md "## " starts a
chapter; "<!-- chapter: GROUP | ICON | COLOUR -->" right after it puts it in a group of the sidebar, with an icon
(ICONS below) and a colour (va, pd, organ, fm6, drums, acid, rec). Its first short paragraph is its lead; each
"### " is a card; "<!-- scene: NAME -->" a scene, as a card of its own. A list whose items all start bold
("- **Open:** ...") shows as key and value.

A scene is a host script. Its first lines up to the first step set things up; "#: TITLE" names it. "## TEXT" starts
a step (more "## " lines right after it: the same text); its commands follow, and at its end the screen is taken.
"#> hear" in a step records what it plays. The controls a step uses (tap, press, release, turn, spin, key, tapkey,
strum, master, midi) are what the picture of the panel shows. A scene runs as a test: an expect that fails stops the
manual. So does a scene missing in MANUAL.md, or one MANUAL.md names without its file.

The glossary is checked against the names the screen shows (the simulator's "labels" command, app.c): each needs an
entry, an alias ("auch: ..."), or a place in the group "Ohne Eintrag". Fractions, numbers and "-" need none. Without
--strict what is missing is only reported; with it (the CI) the run fails.
"""
import html
import json
import re
import shutil
import subprocess
import sys
import tempfile
from datetime import date
from pathlib import Path

APP = Path(__file__).resolve().parents[1]
ROOT = APP.parents[1]
DOCS = APP / "docs"
SCENES = DOCS / "scenes"
ASSETS = DOCS / "manual"
FONTS = ROOT / "base" / "assets" / "fonts"
HOST = APP / "build" / "host" / "klassik_host"
OUT = APP / "build" / "manual"

ENCODERS = ("SELECT", "ALGO", "PRESET", "K1", "K2", "K3", "K4")
BUTTONS = ("FX", "SEL", "ENV", "LFO", "EDIT", "GLO", "HOME", "SAVE", "ARP", "SEQ", "PLAY", "REC", "OCT-", "OCT+")
WHITE_K = (0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26)   # key index (F3 = 0) of each white key
BLACK_K = (1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25)
SELF_EVIDENT = re.compile(r"^(-|\d+|\d+/\d+[.T]?|\d+ BARS?)$")         # needs no entry: fractions, numbers
COLOURS = ("va", "pd", "organ", "fm6", "drums", "acid", "rec")
SCENE = re.compile(r"^<!--\s*scene:\s*([\w-]+)\s*-->\s*$")
CHAPTER = re.compile(r"^<!--\s*chapter:\s*(.*?)\s*-->\s*$")

# line icons, 24 x 24, drawn with the stroke
ICONS = {
    "home": "M3 11 12 4l9 7v9a1 1 0 0 1-1 1h-5v-6H9v6H4a1 1 0 0 1-1-1z",
    "parts": "M12 3 2 8l10 5 10-5zM2 12.5l10 5 10-5M2 17l10 5 10-5",
    "rack": "M3 4h7v7H3zM14 4h7v7h-7zM3 13h7v7H3zM14 13h7v7h-7z",
    "mixer": "M6 3v18M12 3v18M18 3v18M3.5 8h5M9.5 15h5M15.5 10h5",
    "list": "M9 6h12M9 12h12M9 18h12M4 6h.01M4 12h.01M4 18h.01",
    "fx": "M2 12c2.2-6 4.3-6 6.5 0s4.3 6 6.5 0 4.3-6 6.5 0M19 3l.8 1.7L21.5 5.5l-1.7.8L19 8l-.8-1.7-1.7-.8 1.7-.8z",
    "lfo": "M2 12c2.5-7 5-7 7.5 0s5 7 7.5 0c1.2-3.4 2.5-5 5-5",
    "arp": "M3 20h4v-4h4v-4h4V8h4V4h2",
    "loop": "M17 2l3 3-3 3M3 11V9a4 4 0 0 1 4-4h13M7 22l-3-3 3-3M21 13v2a4 4 0 0 1-4 4H4",
    "step": "M3 8h4v4H3zM10 8h4v4h-4zM17 8h4v4h-4zM3 15h4v4H3zM17 15h4v4h-4zM10 15h4v4h-4z",
    "edit": "M12 21a8 8 0 1 0 0-16 8 8 0 0 0 0 16zM12 9v4.5M5 3l2 2M19 3l-2 2",
    "env": "M2 20 7 5l4 7h6l5 8",
    "fm6": "M3 3h6v6H3zM15 3h6v6h-6zM9 15h6v6H9zM6 9v3h12V9M12 12v3",
    "drums": "M4 8c0-1.8 3.6-3 8-3s8 1.2 8 3-3.6 3-8 3-8-1.2-8-3zM4 8v8c0 1.8 3.6 3 8 3s8-1.2 8-3V8M15 3l4-1",
    "acid": "M2 17 8 6v11l6-11v11l6-11v11",
    "book": "M4 4h6a3 3 0 0 1 3 3v14a2 2 0 0 0-2-2H4zM20 4h-5a3 3 0 0 0-3 3v14a2 2 0 0 1 2-2h6z",
    "search": "M11 18a7 7 0 1 0 0-14 7 7 0 0 0 0 14zM20 20l-3.5-3.5",
    "va": "M2 17 7 7v10l5-10v10M15 6c3.5 0 4.5 2 7 11",
    "pd": "M2 12c1.5-5 3-7 4.5-7S8 18 12 18s4-13 6-13 2.5 4 4 7",
    "organ": "M4 3v9M8 3v15M12 3v6M16 3v18M20 3v11M3 12h2M7 18h2M11 9h2M15 21h2M19 14h2",
    "dot": "M12 12h.01",
}


class Fail(Exception):
    pass


def icon(name, cls="ico"):
    d = ICONS.get(name, ICONS["dot"])
    return (f'<svg class="{cls}" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" '
            f'stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="{d}"/></svg>')


def logo():
    """A glyph, as the device draws one: eight spokes from the top, clockwise, filled 3/8, a centre dot."""
    import math
    r = (.95, .42, .78, .6, .88, .35, .7, .52)
    pts = " ".join(f"{20 + 15 * v * math.sin(i * math.pi / 4):.1f},{20 - 15 * v * math.cos(i * math.pi / 4):.1f}"
                   for i, v in enumerate(r))
    return (f'<svg class="logo" viewBox="0 0 40 40" aria-hidden="true"><polygon points="{pts}" fill="#ffb454" '
            f'fill-opacity=".38" stroke="#ffb454" stroke-width="2" stroke-linejoin="round"/>'
            f'<rect x="18.5" y="18.5" width="3" height="3" fill="#ffb454"/></svg>')


# ------------------------------------------------------------------------------------------------ markdown, a subset
def slug(s):
    """A heading's anchor as GitHub makes it: lower case, punctuation gone, spaces to hyphens."""
    return re.sub(r"[^\w\- ]", "", s.strip().lower()).replace(" ", "-")


def term_id(t):
    """A glossary entry's anchor: OCT+ and OCT- stay apart."""
    sub = {"+": "plus", "&": "and", ">": "to", "/": "-", " ": "-", ".": "dot", "−": "-"}
    return "g-" + "".join(sub.get(c, c) for c in t.lower() if c.isalnum() or c in sub or c == "-")


def link_target(url):
    """Links between the two files work on GitHub and on the page."""
    m = re.match(r"^(?:MANUAL|GLOSSARY)\.md(#.*)?$", url)
    if m:
        return m.group(1) or "#start"
    return url


def inline(s, images=None):
    """Code spans, then images, links, bold and italics; the rest escaped."""
    out = []
    for i, part in enumerate(re.split(r"(`[^`]*`)", s)):
        if i % 2:
            out.append("<code>" + html.escape(part[1:-1]) + "</code>")
            continue
        t = html.escape(part, quote=False)

        def img(m):
            src = m.group(2)
            if images is not None:
                images.add(src)
            if src.endswith(".svg") and (DOCS / src).exists():   # inline: the page's font reaches it
                svg = (DOCS / src).read_text(encoding="utf-8")
                svg = re.sub(r"^<svg ", f'<svg role="img" aria-label="{m.group(1)}" class="pic" ', svg.strip())
                return re.sub(r"<!--.*?-->", "", svg, flags=re.S)
            return f'<img src="{html.escape(Path(src).name)}" alt="{m.group(1)}" class="pic">'
        t = re.sub(r"!\[([^\]]*)\]\(([^)\s]+)\)", img, t)
        t = re.sub(r"\[([^\]]+)\]\(([^)\s]+)\)",
                   lambda m: f'<a href="{html.escape(link_target(m.group(2)))}">{m.group(1)}</a>', t)
        t = re.sub(r"\*\*(.+?)\*\*", r"<strong>\1</strong>", t)
        t = re.sub(r"(?<![\w*])\*(?!\s)(.+?)(?<!\s)\*(?![\w*])", r"<em>\1</em>", t)
        out.append(t)
    return "".join(out)


def caps(cell, images=None):
    """A table's first column: the controls as key caps (HOME + K1–K4, K3, K4)."""
    parts = []
    for piece in re.split(r"\s+\+\s+", cell):
        parts.append(" ".join(f'<kbd>{inline(x, images)}</kbd>' for x in re.split(r",\s+", piece)))
    return '<span class="plus">+</span>'.join(parts)


def blocks(lines, heading=None, images=None):
    """Markdown lines to HTML: headings, paragraphs, lists, tables, quotes (a box)."""
    out, i, n = [], 0, len(lines)
    while i < n:
        line = lines[i]
        if not line.strip():
            i += 1
            continue
        if line.startswith("<!--"):
            while i < n and "-->" not in lines[i]:
                i += 1
            i += 1
            continue
        m = re.match(r"^(#{1,4}) (.*)$", line)
        if m:
            level, text = len(m.group(1)), m.group(2).strip()
            hid = heading(text) if heading else slug(text)
            out.append(f'<h{level} id="{hid}">{inline(text, images)}</h{level}>')
            i += 1
            continue
        if line.startswith("|"):
            rows = []
            while i < n and lines[i].startswith("|"):
                rows.append([c.strip() for c in lines[i].strip().strip("|").split("|")])
                i += 1
            head, body = rows[0], rows[2:]
            t = ["<div class='tbl'><table><thead><tr>" + "".join(f"<th>{inline(c, images)}</th>" for c in head)
                 + "</tr></thead><tbody>"]
            for r in body:
                t.append("<tr>" + "".join(f"<td>{caps(c, images) if k == 0 else inline(c, images)}</td>"
                                          for k, c in enumerate(r)) + "</tr>")
            out.append("".join(t) + "</tbody></table></div>")
            continue
        if re.match(r"^(- |\d+\. )", line):
            tag = "ol" if line[0].isdigit() else "ul"
            items = []
            while i < n and lines[i].strip():
                if re.match(r"^(- |\d+\. )", lines[i]):
                    items.append(re.sub(r"^(- |\d+\. )", "", lines[i]))
                elif lines[i].startswith("  ") and items:
                    items[-1] += " " + lines[i].strip()
                else:
                    break
                i += 1
            keyed = [re.match(r"^\*\*([^*]+?):?\*\*:?\s*(.*)$", x) for x in items]
            if tag == "ul" and all(keyed):
                out.append('<ul class="keys">' + "".join(          # a list of names (Presets) gets no term links
                    f'<li{" class=nolink" if m.group(1).startswith("Presets") else ""}><span class="k">'
                    f'{inline(m.group(1), images)}</span><span class="v">{inline(m.group(2), images)}'
                    f'</span></li>' for m in keyed) + "</ul>")
            else:
                out.append(f"<{tag}>" + "".join(f"<li>{inline(x, images)}</li>" for x in items) + f"</{tag}>")
            continue
        if line.startswith(">"):
            q = []
            while i < n and lines[i].startswith(">"):
                q.append(lines[i][1:].lstrip() if len(lines[i]) > 1 else "")
                i += 1
            out.append('<aside class="note">' + blocks(q, heading, images) + "</aside>")
            continue
        p = []
        while i < n and lines[i].strip() and not re.match(r"^(#{1,4} |\||- |\d+\. |>|<!--)", lines[i]):
            p.append(lines[i].strip())
            i += 1
        if len(p) == 1 and re.fullmatch(r"!\[[^\]]*\]\([^)]+\)", p[0]):
            out.append("<figure class='pic'>" + inline(p[0], images) + "</figure>")
        else:
            out.append("<p>" + inline(" ".join(p), images) + "</p>")
    return "\n".join(out)


# ------------------------------------------------------------------------------------------------ the chapters
def parse_manual(lines):
    """MANUAL.md into chapters: title, group, icon, colour, lead, and segments (cards with their lines, scenes)."""
    chapters, cur, seg = [], None, None
    for line in lines[1:] if lines and lines[0].startswith("# ") else lines:
        if line.startswith("## "):
            cur = {"title": line[3:].strip(), "group": "Weitere", "icon": "dot", "colour": "va", "lead": [],
                   "segs": [], "open": True}
            chapters.append(cur)
            seg = None
            continue
        if cur is None:
            continue
        m = CHAPTER.match(line)
        if m:
            f = [x.strip() for x in m.group(1).split("|")]
            cur["group"] = f[0] or cur["group"]
            if len(f) > 1 and f[1]:
                cur["icon"] = f[1]
            if len(f) > 2 and f[2]:
                if f[2] not in COLOURS:
                    raise Fail(f"MANUAL.md, chapter {cur['title']}: colour {f[2]} (one of {', '.join(COLOURS)})")
                cur["colour"] = f[2]
            continue
        m = SCENE.match(line)
        if m:
            cur["segs"].append({"kind": "scene", "name": m.group(1)})
            cur["open"], seg = False, None
            continue
        if line.startswith("### "):
            seg = {"kind": "card", "title": line[4:].strip(), "lines": []}
            cur["segs"].append(seg)
            cur["open"] = False
            continue
        if cur["open"]:
            if not line.strip():
                cur["open"] = not cur["lead"]
                continue
            if cur["lead"] or not re.match(r"^(\||- |\d+\. |>|<!--|!\[|#)", line):
                cur["lead"].append(line.strip())
                continue
            cur["open"] = False
        if seg is None:
            if not line.strip():
                continue
            seg = {"kind": "card", "title": None, "lines": []}
            cur["segs"].append(seg)
        seg["lines"].append(line)
    for c in chapters:                               # a long first paragraph is no lead: a card of its own
        if len(" ".join(c["lead"])) > 320:
            c["segs"].insert(0, {"kind": "card", "title": None, "lines": c["lead"]})
            c["lead"] = []
    return chapters


# ------------------------------------------------------------------------------------------------ the scenes
def parse_scene(path):
    """A scene file into its title, setup and steps (caption, commands, hear)."""
    title, setup, steps = None, [], []
    last_caption = False
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.rstrip()
        if line.startswith("#:"):
            title = line[2:].strip()
            continue
        if line.startswith("## "):
            if last_caption:
                steps[-1]["cap"] += " " + line[3:].strip()
            else:
                steps.append({"cap": line[3:].strip(), "cmds": [], "hear": False})
            last_caption = True
            continue
        last_caption = False
        if line.startswith("#>"):
            if not steps:
                raise Fail(f"{path.name}: '#>' before the first step")
            if line[2:].strip() == "hear":
                steps[-1]["hear"] = True
            else:
                raise Fail(f"{path.name}: unknown '{line}'")
            continue
        (steps[-1]["cmds"] if steps else setup).append(line)
    if not title:
        raise Fail(f"{path.name}: no '#: TITLE'")
    if not steps:
        raise Fail(f"{path.name}: no step ('## TEXT')")
    return title, setup, steps


def key_index(k):
    if k.startswith("w"):
        return WHITE_K[int(k[1:])]
    if k.startswith("b"):
        return BLACK_K[int(k[1:])]
    return int(k)


def actions(cmds, held):
    """What a step does with the panel: the controls it uses, and those held down at its end."""
    act, keys, chans = [], [], set()
    for c in cmds:
        w = c.split()
        if not w or w[0].startswith("#"):
            continue
        op = w[0]
        if op in ("tap", "press", "release") and w[1] in BUTTONS:
            prev = next((a for a in act if a["t"] == "btn" and a["n"] == w[1] and a["how"] == op), None)
            if prev and op == "tap":
                prev["times"] = prev.get("times", 1) + 1
            elif not prev:
                act.append({"t": "btn", "n": w[1], "how": op})
            if op == "press":
                held.add(w[1])
            elif op == "release":
                held.discard(w[1])
        elif op in ("turn", "spin") and w[1] in ENCODERS:
            n = int(w[2])
            prev = next((a for a in act if a["t"] == "enc" and a["n"] == w[1]), None)
            if prev:
                prev["d"] += n
            else:
                act.append({"t": "enc", "n": w[1], "d": n})
        elif op == "master":
            act.append({"t": "enc", "n": "MASTER", "v": round(int(w[1]) * 100 / 4096)})
        elif op == "tapkey" or (op == "key" and len(w) > 2 and w[2] == "down"):
            keys.append(key_index(w[1]))
        elif op == "strum":
            a, b = int(w[1]), int(w[2])
            keys += [WHITE_K[j] for j in range(min(a, b), max(a, b) + 1)]
        elif op == "midi":
            b = w[1:] if w[1] not in ("USB", "BLE", "BLE-DEV", "TRS") else w[2:]
            st = int(b[0], 16)
            if 0x90 <= st <= 0x9F and int(b[2], 16):
                chans.add((st & 15) + 1)
    if keys:
        act.append({"t": "key", "n": sorted(set(keys))})
    if chans:
        act.append({"t": "midi", "ch": sorted(chans)})
    return act, sorted(held)


def run_scene(name, path, work, img, snd):
    title, setup, steps = parse_scene(path)
    script, held, data = list(setup), set(), []
    for k, st in enumerate(steps, 1):
        shot = f"{name}-{k}.png"
        if st["hear"]:
            script.append(f"wav {name}-{k}.wav")
        script += st["cmds"]
        if st["hear"]:
            script.append("wavstop")
        script.append(f"shot {shot}")
        act, now_held = actions(st["cmds"], held)
        data.append({"cap": inline(st["cap"]), "img": f"img/{shot}", "act": act, "held": now_held,
                     "snd": f"snd/{name}-{k}.wav" if st["hear"] else None})
    d = work / name
    d.mkdir(parents=True)
    (d / "scene.txt").write_text("\n".join(script) + "\n", encoding="utf-8")
    r = subprocess.run([str(HOST), str(d / "scene.txt"), str(d)], capture_output=True, text=True)
    if r.returncode:
        bad = [x for x in r.stdout.splitlines() if not x.startswith("  ok")]
        raise Fail(f"scene {name}: the simulator says\n  " + "\n  ".join(bad[-8:]))
    from PIL import Image
    for k, st in enumerate(steps, 1):
        Image.open(d / f"{name}-{k}.png").save(img / f"{name}-{k}.png", optimize=True)
        if st["hear"]:
            shutil.copy(d / f"{name}-{k}.wav", snd / f"{name}-{k}.wav")
    return {"name": name, "title": title, "steps": data, "keys": any(a["t"] == "key" for s in data for a in s["act"])}


def scene_html(sc):
    """The scene as a card; without the script every step in turn (manual.js makes a stepper of it)."""
    li = []
    for k, st in enumerate(sc["steps"], 1):
        audio = f'<audio controls preload="none" src="{st["snd"]}"></audio>' if st["snd"] else ""
        li.append(f'<li><img src="{st["img"]}" alt="Display, Schritt {k}" width="240" height="240">'
                  f'<p>{st["cap"]}</p>{audio}</li>')
    n, s = len(sc["steps"]), sum(1 for st in sc["steps"] if st["snd"])
    meta = f"{n} Schritte" + (f" · {s} zum Anhören" if s else "")
    return (f'<figure class="card scene" id="scene-{sc["name"]}" data-scene="{sc["name"]}">'
            f'<header><span class="tag">Schritt für Schritt</span><h3>{html.escape(sc["title"])}</h3>'
            f'<span class="meta">{meta}</span></header><ol class="steps">{"".join(li)}</ol></figure>')


# ------------------------------------------------------------------------------------------------ the glossary
def parse_glossary(text):
    """Groups of entries: term, aliases, chapter links, body (markdown); the names needing none."""
    groups, ignore, cur, entry = [], set(), None, None
    for line in text.splitlines():
        if line.startswith("## "):
            cur = {"name": line[3:].strip(), "entries": []}
            groups.append(cur)
            entry = None
            continue
        if cur is None:
            continue
        if cur["name"].startswith("Ohne Eintrag"):
            ignore.update(re.findall(r"`([^`]+)`", line))
            continue
        if line.startswith("### "):
            entry = {"term": line[4:].strip(), "aliases": [], "values": [], "links": [], "body": []}
            cur["entries"].append(entry)
            continue
        if entry is None:
            continue
        if not entry["body"] and line.startswith("*") and line.rstrip().endswith("*") and len(line.strip()) > 2:
            for part in line.strip()[1:-1].split(" · "):
                if part.startswith("auch:"):
                    entry["aliases"] += [a.strip().strip("`") for a in part[5:].split(",") if a.strip()]
                if part.startswith("Werte:"):
                    entry["values"] += [a.strip().strip("`") for a in part[6:].split(",") if a.strip()]
                entry["links"] += re.findall(r"\[([^\]]+)\]\(([^)]+)\)", part)
            continue
        entry["body"].append(line)
    return groups, ignore


def glossary_html(groups):
    out, data, letters = [], [], []
    every = [(g, e) for g in groups if not g["name"].startswith("Ohne Eintrag") for e in g["entries"]]
    for g, e in sorted(every, key=lambda ge: ge[1]["term"].lower()):   # A to Z; the group a label and a filter
        first = e["term"][0].upper()
        letter = first if first.isalpha() else "#"
        if not letters or letters[-1] != letter:
            letters.append(letter)
            out.append(f'<h3 class="letter" id="abc-{letter}">{letter}</h3>')
        body = blocks(e["body"])
        links = " ".join(f'<a class="chip" href="{html.escape(link_target(u))}">{html.escape(t)}</a>'
                         for t, u in e["links"])
        aka = " ".join(f"<code>{html.escape(a)}</code>" for a in e["aliases"])
        vals = " ".join(f"<code>{html.escape(a)}</code>" for a in e["values"])
        tid = term_id(e["term"])
        out.append(f'<article class="entry" id="{tid}" data-kind="{html.escape(g["name"])}" data-letter="{letter}">'
                   f'<header><span class="term">{html.escape(e["term"])}</span>'
                   f'<span class="kind">{html.escape(g["name"])}</span></header>'
                   + (f'<p class="aka">auch {aka}</p>' if aka else "") + body
                   + (f'<p class="aka vals">Werte {vals}</p>' if vals else "")
                   + (f'<p class="see">{links}</p>' if links else "") + "</article>")
        plain = re.sub(r"\s+", " ", re.sub(r"<[^>]+>", "", re.sub(r"</(p|li|td|th|tr|h\d)>", " ", body)))
        short = re.split(r"(?<=[.!?])\s", plain.strip(), maxsplit=1)[0]
        data.append({"id": tid, "term": e["term"], "aliases": e["aliases"], "values": e["values"], "kind": g["name"],
                     "short": html.unescape(short), "text": html.unescape(plain)})
    kinds = [g["name"] for g in groups if not g["name"].startswith("Ohne Eintrag")]
    chips = "".join(f'<button type="button" data-kind="{html.escape(k)}">{html.escape(k)}</button>' for k in kinds)
    abc = "".join(f'<a href="#abc-{x}">{x}</a>' for x in letters)
    return (f'<section class="space glossary" id="glossar" data-colour="va" data-title="Glossar">'
            f'<header class="hero"><div class="hero-ico">{icon("book")}</div>'
            f'<p class="crumb">Nachschlagen</p><h2>Glossar</h2>'
            f'<p class="lead">Jeder Begriff und jedes Kürzel vom Display, kurz erklärt. Im Text sind sie unterstrichen: '
            f'Zeigen genügt.</p></header>'
            f'<div class="gbar"><div class="field">{icon("search", "ico s")}'
            f'<input type="search" id="gsearch" placeholder="Begriff, Kürzel oder Erklärung …" '
            f'aria-label="Im Glossar suchen" autocomplete="off"></div>'
            f'<div class="chips"><button type="button" data-kind="" aria-pressed="true">Alle</button>{chips}</div>'
            f'<nav class="abc" aria-label="Alphabet">{abc}</nav><p class="gcount" aria-live="polite"></p></div>'
            f'<div class="entries">{"".join(out)}</div></section>'), data


# ------------------------------------------------------------------------------------------------ the checks
def labels(work):
    """The names the screen shows, as the simulator lists them: {text: kind}."""
    s = work / "labels.txt"
    s.write_text("labels\n", encoding="utf-8")
    r = subprocess.run([str(HOST), str(s), str(work)], capture_output=True, text=True)
    if r.returncode:
        raise Fail("labels: the simulator failed\n" + r.stdout[-500:])
    got = {}
    for line in r.stdout.splitlines():
        m = re.match(r"^label (\w+) (.+)$", line)
        if m:
            got.setdefault(m.group(2), m.group(1))
    if len(got) < 50:
        raise Fail("labels: the simulator listed too few names (labels in app.c?)")
    return got


def coverage(names, groups, ignore):
    have = set(ignore)
    for g in groups:
        for e in g["entries"]:
            have.add(e["term"])
            have.update(e["aliases"])
            have.update(e["values"])
    missing = {}
    for t, kind in names.items():
        if SELF_EVIDENT.match(t) or t in have:
            continue
        m = re.match(r"^K[1-4] (.+)$", t)
        if m and m.group(1) in have:
            continue
        missing.setdefault(kind, []).append(t)
    return missing


# ------------------------------------------------------------------------------------------------ the page
def git_state():
    try:
        h = subprocess.run(["git", "rev-parse", "--short", "HEAD"], cwd=ROOT, capture_output=True, text=True).stdout
        d = subprocess.run(["git", "status", "--porcelain", "--", "firmware/klassik"], cwd=ROOT, capture_output=True,
                           text=True).stdout
        return h.strip() + ("+" if d.strip() else "")
    except OSError:
        return ""


def build_chapters(chapters, files, work, img, snd, images):
    """Each chapter as a space: its hero, its cards and scenes. Returns the HTML, the scenes, the anchors."""
    seen, used, scenes, spaces = {}, [], [], []

    def anchor(text):
        h = slug(text)
        if h in seen:
            seen[h] += 1
            h = f"{h}-{seen[h]}"
        else:
            seen[h] = 0
        return h

    seen["start"] = seen["glossar"] = 0
    for n, ch in enumerate(chapters, 1):
        ch["id"] = anchor(ch["title"])
        cards = []
        for seg in ch["segs"]:
            if seg["kind"] == "scene":
                name = seg["name"]
                if name not in files:
                    raise Fail(f"MANUAL.md names scene {name}: no docs/scenes/{name}.txt")
                if name in used:
                    raise Fail(f"MANUAL.md names scene {name} twice")
                used.append(name)
                sc = run_scene(name, files[name], work, img, snd)
                scenes.append(sc)
                cards.append(scene_html(sc))
                continue
            body = blocks(seg["lines"], anchor, images)
            t = seg["title"]
            cls = "card"
            if t == "Auf einen Blick":
                cls += " glance"
            elif t and t.startswith("Grundlagen"):
                cls += " basics"
            elif t and t.startswith("Gut zu wissen"):
                cls += " tips"
            elif t and t.startswith("Presets"):
                cls += " nolink"
            head = f'<header><h3 id="{anchor(t)}">{inline(t)}</h3></header>' if t else ""
            cards.append(f'<article class="{cls}">{head}<div class="body">{body}</div></article>')
        lead = f'<p class="lead">{inline(" ".join(ch["lead"]), images)}</p>' if ch["lead"] else ""
        spaces.append(
            f'<section class="space chapter" id="{ch["id"]}" data-colour="{ch["colour"]}" '
            f'data-title="{html.escape(ch["title"])}">'
            f'<header class="hero"><div class="hero-ico">{icon(ch["icon"])}</div>'
            f'<p class="crumb">{html.escape(ch["group"])} <span>· Kapitel {n}</span></p>'
            f'<h2>{inline(ch["title"])}</h2>{lead}<p class="stats"></p></header>'
            f'<div class="cards">{"".join(cards)}</div><nav class="pager" aria-label="Kapitel"></nav></section>')
    for f in files:
        if f not in used:
            raise Fail(f"docs/scenes/{f}.txt is in no chapter: <!-- scene: {f} --> in MANUAL.md")
    return "".join(spaces), scenes, set(seen)


def side_nav(chapters, n_gloss):
    groups = []
    for ch in chapters:
        if ch["group"] not in groups:
            groups.append(ch["group"])
    out = [f'<a class="item" href="#start" data-colour="va">{icon("home")}<span>Übersicht</span></a>']
    for g in groups:
        out.append(f'<p class="grp">{html.escape(g)}</p>')
        for ch in chapters:
            if ch["group"] == g:
                ns = sum(1 for s in ch["segs"] if s["kind"] == "scene")
                badge = f'<em title="{ns} Szenen">{ns}</em>' if ns else ""
                out.append(f'<a class="item" href="#{ch["id"]}" data-colour="{ch["colour"]}">{icon(ch["icon"])}'
                           f'<span>{inline(ch["title"])}</span>{badge}</a>')
    out.append('<p class="grp">Nachschlagen</p>')
    out.append(f'<a class="item" href="#glossar" data-colour="va">{icon("book")}<span>Glossar</span>'
               f'<em>{n_gloss}</em></a>')
    return "".join(out), groups


def home_html(chapters, groups, problems, n_scenes, n_steps, n_gloss):
    cols = []
    for g in groups:
        cards = []
        for n, ch in enumerate(chapters, 1):
            if ch["group"] != g:
                continue
            ns = sum(1 for s in ch["segs"] if s["kind"] == "scene")
            lead = html.unescape(re.sub(r"<[^>]+>", "", inline(" ".join(ch["lead"]))))
            if not lead:                             # no lead: the chapter's first sentence
                first = next((s for s in ch["segs"] if s["kind"] == "card"), {"lines": []})["lines"]
                text = " ".join(x.strip() for x in first if x.strip() and not x.startswith(("|", "<!--", "#")))
                text = html.unescape(re.sub(r"<[^>]+>", "", inline(text)))
                lead = re.split(r"(?<=[.!?:])\s", text, maxsplit=1)[0][:160]
            cards.append(f'<a class="tile" href="#{ch["id"]}" data-colour="{ch["colour"]}">'
                         f'<span class="tico">{icon(ch["icon"])}</span><span class="tnum">{n:02d}</span>'
                         f'<strong>{inline(ch["title"])}</strong>'
                         f'<span class="tlead">{html.escape(lead) if lead else "&nbsp;"}</span>'
                         + (f'<span class="tmeta">▶ {ns} Szene{"n" if ns != 1 else ""}</span>' if ns else "")
                         + "</a>")
        cols.append(f'<div class="tgroup"><h3>{html.escape(g)}</h3><div class="tiles">{"".join(cards)}</div></div>')
    todo = ""
    if problems:
        todo = ('<details class="card todo"><summary>Stand der Doku: noch offen (die CI prüft das mit '
                '<code>--strict</code>)</summary><ul>' + "".join(f"<li>{html.escape(p)}</li>" for p in problems)
                + "</ul></details>")
    return (f'<section class="space home" id="start" data-colour="va" data-title="Übersicht">'
            f'<header class="hero big"><div class="hero-ico">{logo()}</div><p class="crumb">Handbuch</p>'
            f'<h2>KLASSIK</h2><p class="lead">Sechs Parts, sechs Engines, Mixer, Effekte, Looper: jede Funktion '
            f'Schritt für Schritt, mit echten Bildern aus dem Simulator und zum Anhören.</p>'
            f'<p class="stats"><span>{len(chapters)} Kapitel</span><span>{n_scenes} Szenen · {n_steps} Schritte'
            f'</span><span>{n_gloss} Begriffe im Glossar</span></p></header>'
            f'{"".join(cols)}{todo}</section>')


def page(nav, spaces, data, version):
    data = json.dumps(data, ensure_ascii=False).replace("</", "<\\/")
    return f"""<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>KLASSIK Handbuch</title>
<link rel="stylesheet" href="manual.css">
</head>
<body>
<div class="layout">
<nav class="side" aria-label="Inhalt">
  <a class="brand" href="#start">{logo()}<span><b>KLASSIK</b><small>Handbuch · {html.escape(version)}</small></span></a>
  <div class="search"><svg class="ico s" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"
    stroke-linecap="round" aria-hidden="true"><circle cx="11" cy="11" r="7"/><path d="m20 20-3.5-3.5"/></svg>
    <input type="search" id="q" placeholder="Suchen" aria-label="Handbuch durchsuchen" autocomplete="off">
    <kbd>/</kbd></div>
  <ol id="hits" class="hits" hidden></ol>
  <div class="menu" id="nav">{nav}</div>
  <a class="emu" href="/" target="_blank" rel="noopener" hidden>Im Emulator ausprobieren <span>↗</span></a>
</nav>
<main id="main">
{spaces}
</main>
</div>
<div id="tip" class="tip" role="tooltip" hidden></div>
<script id="data" type="application/json">{data}</script>
<script src="manual.js"></script>
</body>
</html>
"""


def main():
    strict = "--strict" in sys.argv[1:]
    if not HOST.exists():
        raise Fail(f"no simulator at {HOST}: first python3 base/tools/build.py firmware/klassik --host")
    chapters = parse_manual((DOCS / "MANUAL.md").read_text(encoding="utf-8").splitlines())
    order = list(dict.fromkeys(ch["group"] for ch in chapters))   # the groups as they first come; within, the text's
    chapters.sort(key=lambda ch: order.index(ch["group"]))
    groups, ignore = parse_glossary((DOCS / "GLOSSARY.md").read_text(encoding="utf-8"))
    seen_ids = {}
    for g in groups:                                 # one entry a term: two would share an anchor
        for e in g["entries"]:
            tid = term_id(e["term"])
            if tid in seen_ids:
                raise Fail(f"GLOSSARY.md: {e['term']} twice (or as {seen_ids[tid]})")
            seen_ids[tid] = e["term"]

    if OUT.exists():
        shutil.rmtree(OUT)
    img, snd, fonts = OUT / "img", OUT / "snd", OUT / "fonts"
    for d in (img, snd, fonts):
        d.mkdir(parents=True)
    problems, images = [], set()
    with tempfile.TemporaryDirectory() as tmp:
        work = Path(tmp)
        files = {p.stem: p for p in sorted(SCENES.glob("*.txt"))}
        body, scenes, anchors = build_chapters(chapters, files, work, img, snd, images)
        for src in images:
            p = (DOCS / src).resolve()
            if not p.exists():
                raise Fail(f"MANUAL.md: picture {src} not found")
            shutil.copy(p, OUT / p.name)
        names = labels(work)

    for g in groups:
        for e in g["entries"]:
            for t, u in e["links"]:
                a = link_target(u)
                if a.startswith("#") and a[1:] not in anchors:
                    problems.append(f"Glossar {e['term']}: Kapitel-Link {u} findet keine Überschrift")
    missing = coverage(names, groups, ignore)
    n_missing = sum(len(v) for v in missing.values())
    kind_de = {"button": "Taster", "encoder": "Regler", "engine": "Engines", "tab": "Tabs", "page": "Edit-Seiten",
               "param": "Parameter", "value": "Werte", "mark": "Anzeigen"}
    for kind, ts in sorted(missing.items()):
        problems.append(f"Ohne Glossar-Eintrag ({kind_de.get(kind, kind)}): " + ", ".join(sorted(ts)))

    gloss, gdata = glossary_html(groups)
    nav, order = side_nav(chapters, len(gdata))
    steps = sum(len(s["steps"]) for s in scenes)
    home = home_html(chapters, order, problems, len(scenes), steps, len(gdata))
    version = f"{git_state()} · {date.today().strftime('%d.%m.%Y')}"
    data = {"scenes": {s["name"]: s for s in scenes}, "glossary": gdata}
    (OUT / "index.html").write_text(page(nav, home + body + gloss, data, version), encoding="utf-8")
    for f in ("manual.css", "manual.js"):
        shutil.copy(ASSETS / f, OUT / f)
    for f in ("BarlowSemiCondensed-Medium.ttf", "BarlowSemiCondensed-SemiBold.ttf", "BarlowSemiCondensed-Bold.ttf",
              "Barlow-OFL.txt"):
        shutil.copy(FONTS / f, fonts / f)

    print(f"manual: {len(chapters)} chapters, {len(scenes)} scenes ({steps} steps), {len(gdata)} glossary entries")
    print(f"labels: {len(names) - n_missing} of {len(names)} names covered")
    for p in problems:
        print("  " + p)
    print(f"-> {OUT / 'index.html'}")
    if strict and problems:
        print("gen_manual: FAIL (--strict)")
        return 1
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Fail as e:
        print(f"gen_manual: FAIL: {e}")
        sys.exit(1)
