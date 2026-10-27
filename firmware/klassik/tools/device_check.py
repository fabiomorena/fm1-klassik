#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
# Copyright (C) 2026 fabiomorena
"""KLASSIK's device build up to fm1-base's checks, without the package.

  JIELI_TOOLCHAIN=~/.jieli/toolchain .venv/bin/python firmware/klassik/tools/device_check.py

Runs base/tools/build.py's own steps for the FM-1 (the generators, the loader, compile and link with JieLi's
toolchain in Docker, then its checks: the app slot, RAM, the pool, no double, no 64-bit multiply beside a load, no
call from RAM code, register access only in felucca-bsp) and prints them, as build.py does before it packs. It
writes no .fwsc: the package needs the JieLi AC79 SDK's boot files (base/bsp/tools/get_sdk.sh), and a package without
them must never reach a device. Exit 1 when a check fails.
"""
import os
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
APP = REPO / "firmware" / "klassik"
sys.path.insert(0, str(REPO / "base" / "tools"))
import build as B  # noqa: E402  (fm1-base's build, used as it is)


def main():
    B.APP = APP
    B.CONF = B.read_conf(APP / "fm1.toml")
    B.NAME = B.STEM = B.CONF.get("name", APP.name).lower()
    B.BLE, B.WIFI = bool(B.CONF.get("ble")), bool(B.CONF.get("wifi"))
    B.CPU2 = bool(B.CONF.get("cpu2")) and os.environ.get("FM1B_CPU2") != "0"
    B.OUT = APP / "build"
    B.GEN, B.LDR = B.OUT / "gen", B.OUT / "loader"
    B.ROOT = Path(os.path.commonpath([B.SRC, APP]))
    B.PRODUCT = B.CONF.get("id_prefix", "FM-1_0") + "00"
    B.OUT.mkdir(parents=True, exist_ok=True)
    B.generate()
    ota = B.build_loader()
    img, syms, dis, rt = B.build_app()
    errors, notes = B.check(img, syms, dis, rt)
    errors += B.mmio_check()
    for n in notes:
        print("  ok   ", n)
    for e in errors:
        print("  FAIL ", e)
    print(f"app      {len(img)} B of the {B.APP_SLOT} B slot; loader {len(ota)} B; no package written")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
