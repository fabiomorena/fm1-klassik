#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
# Copyright (C) 2026 fabiomorena
"""KLASSIK's browser emulator on http://localhost:8000, as `python3 -m http.server` serves it, but never cached.

  python3 firmware/klassik/tools/serve_emu.py [PORT]

The emulator's page fetches app.wasm only once "Switch on" is clicked, past a hard reload's reach; served without
cache headers, Chrome keeps the old app.wasm after a rebuild. Here every answer says Cache-Control: no-store, so a
reload shows the build as it is (a browser that kept one from before needs its cache emptied once).
"""
import functools
import http.server
import sys
from pathlib import Path

EMU = Path(__file__).resolve().parents[1] / "build" / "emu"


class NoCache(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cache-Control", "no-store")
        super().end_headers()


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    if not (EMU / "app.wasm").exists():
        sys.exit(f"serve_emu: no emulator in {EMU} (base/tools/build.py firmware/klassik --web)")
    print(f"serve_emu: http://localhost:{port} from {EMU}", flush=True)
    http.server.ThreadingHTTPServer(("", port), functools.partial(NoCache, directory=str(EMU))).serve_forever()


if __name__ == "__main__":
    main()
