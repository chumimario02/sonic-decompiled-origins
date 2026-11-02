#!/usr/bin/env bash
# Compila el lanzador. En la Pi necesita libsdl2-dev.
set -euo pipefail
cd "$(dirname "$0")"
cc -O2 -std=c11 launcher.c -o launcher $(pkg-config --cflags --libs sdl2)
echo "OK: launcher/launcher"