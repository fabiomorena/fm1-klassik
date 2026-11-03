// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2026 fabiomorena
// KLASSIK's manual (tools/gen_manual.py writes the page): a space for each chapter, the overview and the glossary,
// one shown at a time (the address's #); the scenes as steppers with a picture of the panel; the search; the
// glossary's filter; the glossary's terms linked in the text with a short explanation on hover.
"use strict";
const DATA = JSON.parse(document.getElementById("data").textContent);
const NS = "http://www.w3.org/2000/svg";
const $ = (s, r = document) => r.querySelector(s);
const $$ = (s, r = document) => [...r.querySelectorAll(s)];
const spaces = $$(".space");
const navItems = $$("#nav a.item");

if (location.protocol.startsWith("http")) $(".emu").hidden = false;   // the emulator: the same server's root

// ------------------------------------------------------------------------------------------------ the panel
// The layout after fm1-base's emulator page (base/web/emu/index.html, MPL-2.0): a 1000 x 640 drawing, the labels
// larger so that they stay legible small, the names as the manual uses them (PRESET, ALGO, K1..K4).
const KNOBS = { MASTER: [105, 100, 24], SELECT: [215, 100, 24], PRESET: [105, 205, 24], ALGO: [215, 205, 24],
                K1: [610, 100, 22], K2: [714, 100, 22], K3: [818, 100, 22], K4: [922, 100, 22] };
const ROW1 = ["FX", "SEL", "ENV", "LFO", "EDIT", "GLO"], ROW2 = ["HOME", "SAVE", "ARP", "SEQ", "PLAY", "REC"];
const WHITE_K = [0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26];
const BLACK_K = [1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25];
const BLACK_AFTER = [0, 1, 2, 4, 5, 7, 8, 9, 11, 12, 14];
const NOTE = ["F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E"];
const keyName = (k) => NOTE[k % 12] + (3 + Math.floor((k + 5) / 12));

function el(tag, attrs, parent) {
  const e = document.createElementNS(NS, tag);
  for (const [k, v] of Object.entries(attrs)) e.setAttribute(k, v);
  if (parent) parent.appendChild(e);
  return e;
}
function label(parent, x, y, s, size, cls = "lab") {
  const t = el("text", { x, y, "text-anchor": "middle", "font-size": size, class: cls }, parent);
  t.textContent = s;
  return t;
}

function makePanel(withKeys) {
  const svg = el("svg", { viewBox: withKeys ? "0 0 1000 640" : "0 0 1000 340", class: "panel", role: "img" });
  const p = { svg, knobs: {}, btns: {}, keys: [] };
  const defs = el("defs", {}, svg);
  const g1 = el("linearGradient", { id: "pb", x1: 0, y1: 0, x2: 0, y2: 1 }, defs);
  el("stop", { offset: 0, "stop-color": "#464a51" }, g1);
  el("stop", { offset: 1, "stop-color": "#2f3238" }, g1);
  el("rect", { x: 4, y: 4, width: 992, height: withKeys ? 632 : 332, rx: 44, fill: "url(#pb)", class: "body" }, svg);
  for (const [n, [x, y, r]] of Object.entries(KNOBS)) {
    const g = el("g", { class: "ctl knob" }, svg);
    el("circle", { cx: x, cy: y, r: r + 5, class: "tray" }, g);
    el("circle", { cx: x, cy: y, r, class: "face" }, g);
    el("line", { x1: x, y1: y - r + 5, x2: x, y2: y - r + 15, class: "ind" }, g);
    label(g, x, y - r - 14, n, 22);
    p.knobs[n] = { g, x, y, r };
  }
  const button = (x, y, w, h, n) => {
    const g = el("g", { class: "ctl btn" }, svg);
    el("rect", { x, y, width: w, height: h, rx: 9, class: "face" }, g);
    label(g, x + w / 2, y + h / 2 + 7, n, n.length > 3 ? 18 : 21);
    p.btns[n] = { g, x, y, w, h };
  };
  el("rect", { x: 58, y: 258, width: 210, height: 58, rx: 14, class: "tray" }, svg);
  button(72, 268, 82, 38, "OCT-");
  button(170, 268, 82, 38, "OCT+");
  el("rect", { x: 300, y: 44, width: 236, height: 236, rx: 26, class: "bezel" }, svg);
  p.screen = el("image", { x: 310, y: 54, width: 216, height: 216, preserveAspectRatio: "xMidYMid meet" }, svg);
  el("rect", { x: 568, y: 160, width: 400, height: 140, rx: 18, class: "tray" }, svg);
  ROW1.forEach((n, i) => button(582 + i * 63, 174, 54, 48, n));
  ROW2.forEach((n, i) => button(582 + i * 63, 236, 54, 48, n));
  if (withKeys) {
    const x0 = 36, y0 = 340, w = 928, h = 270, pitch = (w - 24) / 16, kw = pitch * 0.78;
    el("rect", { x: x0, y: y0, width: w, height: h, rx: 18, class: "tray" }, svg);
    BLACK_AFTER.forEach((a, k) => {
      const g = el("g", { class: "ctl key black" }, svg);
      el("rect", { x: x0 + 12 + (a + 1) * pitch - kw / 2, y: y0 + h * 0.06, width: kw, height: h * 0.33, rx: kw / 2,
                   class: "face" }, g);
      p.keys[BLACK_K[k]] = { g };
    });
    for (let i = 0; i < 16; i++) {
      const g = el("g", { class: "ctl key white" }, svg);
      el("rect", { x: x0 + 12 + i * pitch + (pitch - kw) / 2, y: y0 + h * 0.43, width: kw, height: h * 0.52,
                   rx: kw / 2, class: "face" }, g);
      p.keys[WHITE_K[i]] = { g };
    }
  }
  p.badges = el("g", { class: "badges" }, svg);
  return p;
}

function badge(p, x, y, s) {
  const g = el("g", { class: "badge" }, p.badges);
  const w = 18 + s.length * 13;
  el("rect", { x: x - w / 2, y: y - 19, width: w, height: 30, rx: 15 }, g);
  label(g, x, y + 3, s, 21, "btxt");
}

function showStep(p, st) {
  $$(".ctl", p.svg).forEach((g) => g.classList.remove("on", "held"));
  p.badges.replaceChildren();
  p.screen.setAttribute("href", st.img);
  const said = [];
  for (const a of st.act) {
    if (a.t === "btn" && p.btns[a.n]) {
      const b = p.btns[a.n];
      b.g.classList.add("on");
      if (a.times > 1) badge(p, b.x + b.w / 2, b.y - 16, `${a.times}×`);
      said.push({ tap: "drücken", press: "halten", release: "loslassen" }[a.how] + " " + a.n +
                (a.times > 1 ? ` (${a.times}×)` : ""));
    } else if (a.t === "enc" && p.knobs[a.n]) {
      const k = p.knobs[a.n];
      k.g.classList.add("on");
      const s = a.v !== undefined ? `${a.v} %` : (a.d > 0 ? `↻ +${a.d}` : `↺ −${-a.d}`);
      badge(p, k.x, k.y + k.r + 34, s);
      said.push(`${a.n} ${a.v !== undefined ? "auf " + a.v + " %" : "drehen " + (a.d > 0 ? "+" : "−") + Math.abs(a.d)}`);
    } else if (a.t === "key") {
      a.n.forEach((k) => p.keys[k] && p.keys[k].g.classList.add("on"));
      said.push("Taste " + a.n.map(keyName).join(", "));
    } else if (a.t === "midi") {
      badge(p, 418, 318, `MIDI Kanal ${a.ch.join(", ")}`);
      said.push(`MIDI auf Kanal ${a.ch.join(", ")}`);
    }
  }
  for (const n of st.held) if (p.btns[n]) p.btns[n].g.classList.add("held");
  p.svg.setAttribute("aria-label", said.length ? "Bedienfeld: " + said.join(", ") : "Bedienfeld");
}

// ------------------------------------------------------------------------------------------------ the scenes
for (const fig of $$("figure.scene")) {
  const sc = DATA.scenes[fig.dataset.scene];
  if (!sc) continue;
  const list = $("ol.steps", fig);
  const stage = document.createElement("div");
  stage.className = "stage";
  stage.innerHTML = `<div class="display"><img alt="" width="240" height="240"></div>
    <div class="right"><div class="pwrap"></div><div class="capbox"><span class="stepno"></span>
    <p class="cap" aria-live="polite"></p></div><div class="hear"></div>
    <div class="nav"><button type="button" class="prev" aria-label="Schritt zurück">‹</button><span class="dots"></span>
    <button type="button" class="next" aria-label="Nächster Schritt">›</button></div></div>`;
  fig.insertBefore(stage, list);
  list.hidden = true;
  fig.tabIndex = 0;
  const p = makePanel(sc.keys);
  $(".pwrap", stage).appendChild(p.svg);
  const img = $(".display img", stage), cap = $(".cap", stage), hear = $(".hear", stage);
  const dots = sc.steps.map((s, i) => {
    const b = document.createElement("button");
    b.type = "button";
    b.className = "dot";
    b.setAttribute("aria-label", `Schritt ${i + 1}`);
    b.addEventListener("click", () => go(i));
    $(".dots", stage).appendChild(b);
    return b;
  });
  let at = 0, loaded = false;
  function go(i) {
    if (!loaded) { sc.steps.forEach((s) => { new Image().src = s.img; }); loaded = true; }
    at = Math.max(0, Math.min(sc.steps.length - 1, i));
    const st = sc.steps[at];
    img.src = st.img;
    img.alt = `Display, Schritt ${at + 1}`;
    cap.innerHTML = st.cap;
    $(".stepno", stage).textContent = `${at + 1}/${sc.steps.length}`;
    hear.replaceChildren();
    if (st.snd) {
      const a = document.createElement("audio");
      a.controls = true;
      a.preload = "none";
      a.src = st.snd;
      const t = document.createElement("span");
      t.className = "hl";
      t.textContent = "Anhören";
      hear.append(t, a);
    }
    showStep(p, st);
    dots.forEach((d, j) => d.setAttribute("aria-current", j === at ? "step" : "false"));
    $(".prev", stage).disabled = at === 0;
    $(".next", stage).disabled = at === sc.steps.length - 1;
  }
  $(".prev", stage).addEventListener("click", () => go(at - 1));
  $(".next", stage).addEventListener("click", () => go(at + 1));
  fig.addEventListener("keydown", (e) => {
    if (e.target.tagName === "AUDIO") return;
    if (e.key === "ArrowRight") { go(at + 1); e.preventDefault(); }
    if (e.key === "ArrowLeft") { go(at - 1); e.preventDefault(); }
  });
  loaded = true;                                    // the first step now, the others once someone steps
  go(0);
  loaded = false;
}

// ------------------------------------------------------------------------------------------------ terms in the text
const byId = Object.fromEntries(DATA.glossary.map((g) => [g.id, g]));
const names = [];
for (const g of DATA.glossary) for (const t of [g.term, ...g.aliases]) names.push([t, g.id]);
names.sort((a, b) => b[0].length - a[0].length);
const idOf = new Map(names);
const esc = (s) => s.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
const TERM = names.length
  ? new RegExp(`(?<![\\p{L}\\p{N}+\\-−])(${names.map((n) => esc(n[0])).join("|")})(?![\\p{L}\\p{N}+\\-−])`, "gu")
  : null;

function linkTerms(nodes, skipId) {   // the first of each term in nodes
  const done = new Set([skipId]);
  for (const root of nodes) {
    const walk = document.createTreeWalker(root, NodeFilter.SHOW_TEXT, {
      acceptNode: (n) => n.parentElement.closest("a, h1, h2, h3, h4, svg, figure.scene, script, .aka, kbd, .k, .nolink")
        ? NodeFilter.FILTER_REJECT : NodeFilter.FILTER_ACCEPT });
    const texts = [];
    while (walk.nextNode()) texts.push(walk.currentNode);
    for (const t of texts) {
      const s = t.nodeValue;
      let m, last = 0, frag = null;
      TERM.lastIndex = 0;
      while ((m = TERM.exec(s))) {
        const id = idOf.get(m[1]);
        if (done.has(id)) continue;
        done.add(id);
        frag = frag || document.createDocumentFragment();
        frag.append(s.slice(last, m.index));
        const a = document.createElement("a");
        a.href = "#" + id;
        a.className = "tlink";
        a.dataset.g = id;
        a.textContent = m[1];
        frag.append(a);
        last = m.index + m[1].length;
      }
      if (frag) {
        frag.append(s.slice(last));
        t.replaceWith(frag);
      }
    }
  }
}
if (TERM) {
  for (const sp of $$(".space.chapter")) linkTerms($$(".lead, .card:not(.scene) .body", sp), null);
  for (const e of $$(".entry")) linkTerms($$(":scope > p:not(.aka):not(.see), :scope > ul, :scope > table", e), e.id);
}

const tip = $("#tip");
function showTip(a) {
  const g = byId[a.dataset.g];
  if (!g) return;
  tip.innerHTML = "";
  const b = document.createElement("strong");
  b.textContent = g.term;
  const k = document.createElement("em");
  k.textContent = g.kind;
  const s = document.createElement("span");
  s.textContent = g.short;
  tip.append(b, k, s);
  tip.hidden = false;
  const r = a.getBoundingClientRect();
  const x = Math.min(window.innerWidth - tip.offsetWidth - 12, Math.max(12, r.left));
  tip.style.left = x + "px";
  tip.style.top = r.bottom + 8 + "px";
}
document.addEventListener("mouseover", (e) => { const a = e.target.closest("a.tlink"); if (a) showTip(a); });
document.addEventListener("mouseout", (e) => { if (e.target.closest("a.tlink")) tip.hidden = true; });
document.addEventListener("focusin", (e) => { const a = e.target.closest("a.tlink"); if (a) showTip(a); else tip.hidden = true; });
document.addEventListener("scroll", () => { tip.hidden = true; }, true);

// ------------------------------------------------------------------------------------------------ the glossary
const gq = $("#gsearch"), entries = $$(".entry"), letters = $$(".glossary .letter");
let gkind = "";
const words = (q) => q.toLowerCase().split(/\s+/).filter(Boolean);
const hay = (g) => [g.term, ...g.aliases, ...(g.values || []), g.text].join(" ").toLowerCase();
function filterGlossary() {
  const w = words(gq.value);
  let n = 0;
  const seen = new Set();
  for (const e of entries) {
    const g = byId[e.id], h = hay(g);
    const ok = (!gkind || g.kind === gkind) && w.every((x) => h.includes(x));
    e.hidden = !ok;
    if (ok) { n++; seen.add(e.dataset.letter); }
  }
  for (const l of letters) l.hidden = !seen.has(l.textContent);
  for (const a of $$(".abc a")) a.classList.toggle("off", !seen.has(a.textContent));
  $(".gcount").textContent = `${n} von ${entries.length}`;
}
gq.addEventListener("input", filterGlossary);
for (const c of $$(".chips button")) c.addEventListener("click", () => {
  gkind = c.dataset.kind;
  $$(".chips button").forEach((b) => b.setAttribute("aria-pressed", b === c ? "true" : "false"));
  filterGlossary();
});
filterGlossary();

// ------------------------------------------------------------------------------------------------ the spaces
const chapters = spaces.filter((s) => s.classList.contains("chapter"));
for (const sp of chapters) {                       // the hero's numbers, the pager
  const sc = $$("figure.scene", sp), snd = sc.reduce((n, f) => n + DATA.scenes[f.dataset.scene].steps
    .filter((s) => s.snd).length, 0), terms = $$("a.tlink", sp).length;
  const parts = [];
  if (sc.length) parts.push(`${sc.length} Szene${sc.length > 1 ? "n" : ""}`);
  if (snd) parts.push(`${snd} zum Anhören`);
  if (terms) parts.push(`${terms} Begriffe`);
  $(".stats", sp).innerHTML = parts.map((x) => `<span>${x}</span>`).join("");
  const i = chapters.indexOf(sp), pager = $(".pager", sp);
  const link = (c, cls, dir) => c ? `<a class="${cls}" href="#${c.id}" data-colour="${c.dataset.colour}">
    <small>${dir}</small><b>${c.dataset.title}</b></a>` : "<span></span>";
  pager.innerHTML = link(chapters[i - 1], "prev", "‹ Zurück") + link(chapters[i + 1] || $("#glossar"), "next", "Weiter ›");
}

function show(hash) {
  const id = decodeURIComponent((hash || "").replace(/^#/, "")) || "start";
  const target = document.getElementById(id) || document.getElementById("start");
  const sp = target.closest(".space") || $("#start");
  for (const s of spaces) s.classList.toggle("on", s === sp);
  document.body.dataset.colour = sp.dataset.colour || "va";
  for (const a of navItems) a.classList.toggle("on", a.getAttribute("href") === "#" + sp.id);
  document.title = (sp.id === "start" ? "" : sp.dataset.title + " · ") + "KLASSIK Handbuch";
  tip.hidden = true;
  if (target.classList.contains("entry") && target.hidden) {
    gq.value = "";
    gkind = "";
    $$(".chips button").forEach((b) => b.setAttribute("aria-pressed", b.dataset.kind ? "false" : "true"));
    filterGlossary();
  }
  if (target === sp) window.scrollTo(0, 0);
  else requestAnimationFrame(() => {
    const box = target.closest(".card, .entry") || target;
    box.scrollIntoView({ block: "start" });
    box.classList.remove("flash");
    void box.offsetWidth;
    box.classList.add("flash");
  });
}
window.addEventListener("hashchange", () => show(location.hash));

// ------------------------------------------------------------------------------------------------ the search
function textOf(el) {                              // its words, cell by cell (textContent runs table cells together)
  const out = [], walk = document.createTreeWalker(el, NodeFilter.SHOW_TEXT);
  while (walk.nextNode()) if (!walk.currentNode.parentElement.closest("svg, script")) out.push(walk.currentNode.nodeValue);
  return out.join(" ").replace(/\s+/g, " ").trim();
}
const index = [];
for (const sp of chapters) {
  const title = sp.dataset.title;
  index.push({ id: sp.id, head: title, where: "", text: ($(".lead", sp) || { textContent: "" }).textContent,
               kind: "Kapitel", s: 0 });
  for (const c of $$(".card", sp)) {
    const h = $("h3", c);
    const id = c.id || (h && h.id) || sp.id;
    const text = c.classList.contains("scene")
      ? DATA.scenes[c.dataset.scene].steps.map((s) => s.cap.replace(/<[^>]+>/g, "")).join(" ")
      : textOf($(".body", c));
    index.push({ id, head: h ? h.textContent : title, where: title, text: text.replace(/\s+/g, " "),
                 kind: c.classList.contains("scene") ? "Szene" : "Abschnitt", s: 1 });
  }
}
const q = $("#q"), hits = $("#hits"), nav = $("#nav");
function snippet(text, w) {
  const i = text.toLowerCase().indexOf(w[0]);
  if (i < 0) return text.slice(0, 90);
  return (i > 30 ? "… " : "") + text.slice(Math.max(0, i - 30), i + 70) + " …";
}
function search() {
  const w = words(q.value);
  hits.replaceChildren();
  hits.hidden = !w.length;
  nav.hidden = !!w.length;
  if (!w.length) return;
  const found = [];
  for (const g of DATA.glossary) {
    const t = g.term.toLowerCase(), h = hay(g);
    if (!w.every((x) => h.includes(x))) continue;
    const al = [...g.aliases, ...(g.values || [])].map((a) => a.toLowerCase());
    const s = t === w.join(" ") ? 0 : t.startsWith(w[0]) ? 1 : al.includes(w.join(" ")) ? 1.5 : al.some((a) => a.includes(w[0])) ? 2 : 4;
    found.push({ s, id: g.id, head: g.term, sub: g.short, kind: "Glossar" });
  }
  for (const c of index) {
    const h = (c.head + " " + c.text).toLowerCase();
    if (!w.every((x) => h.includes(x))) continue;
    found.push({ s: (c.head.toLowerCase().includes(w[0]) ? 2.5 : 5) + c.s / 10, id: c.id,
                 head: c.where ? `${c.where} › ${c.head}` : c.head, sub: snippet(c.text, w), kind: c.kind });
  }
  found.sort((a, b) => a.s - b.s);
  for (const f of found.slice(0, 40)) {
    const li = document.createElement("li"), a = document.createElement("a");
    a.href = "#" + f.id;
    a.innerHTML = `<span class="hk"></span><b></b><span class="hs"></span>`;
    a.querySelector(".hk").textContent = f.kind;
    a.querySelector("b").textContent = f.head;
    a.querySelector(".hs").textContent = f.sub;
    li.appendChild(a);
    hits.appendChild(li);
  }
  if (!found.length) hits.innerHTML = "<li class='none'>Nichts gefunden.</li>";
}
q.addEventListener("input", search);
q.addEventListener("keydown", (e) => {
  if (e.key === "Escape") { q.value = ""; search(); }
  if (e.key === "Enter") { const a = $("a", hits); if (a) a.click(); }
  if (e.key === "ArrowDown") { const a = $("a", hits); if (a) { a.focus(); e.preventDefault(); } }
});
hits.addEventListener("keydown", (e) => {
  const li = e.target.closest("li");
  if (e.key === "ArrowDown" && li.nextElementSibling) { $("a", li.nextElementSibling).focus(); e.preventDefault(); }
  if (e.key === "ArrowUp") { (li.previousElementSibling ? $("a", li.previousElementSibling) : q).focus(); e.preventDefault(); }
  if (e.key === "Escape") { q.value = ""; search(); q.focus(); }
});
document.addEventListener("keydown", (e) => {
  if (e.key === "/" && !/INPUT|TEXTAREA/.test(document.activeElement.tagName)) { q.focus(); e.preventDefault(); }
});

show(location.hash);
