#!/bin/sh
# SPDX-License-Identifier: MPL-2.0
# Copyright (C) 2026 fabiomorena
# One time, in a fresh clone or unpacked copy: the two submodules, at the commits KLASSIK was tested with.
#   sh tools/setup.sh
set -e
cd "$(dirname "$0")/.."
[ -d .git ] || git init -b main
BASE_URL=https://github.com/charlesvestal/fm1-base
BASE_REV=fa7b203e6803a23a8be0bf560c46d07db836ee89          # 2026-10-25
ENG_URL=https://github.com/hugelton/Felucca-Engines
ENG_REV=04ffe903eb87a752305d1e5b10da01b53e4ab004           # 2026-10-16
if [ ! -e base/.git ]; then
    git submodule add "$BASE_URL" base
fi
git -C base fetch --quiet origin "$BASE_REV" 2>/dev/null || true
git -C base checkout --quiet "$BASE_REV"
git -C base submodule update --init --recursive --quiet
if [ ! -e engines/.git ]; then
    git submodule add "$ENG_URL" engines
fi
git -C engines checkout --quiet "$ENG_REV"
echo "setup: base $(git -C base rev-parse --short HEAD), engines $(git -C engines rev-parse --short HEAD)"
echo "next:  python3 base/tools/build.py firmware/klassik --host"
