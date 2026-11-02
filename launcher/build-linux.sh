#!/usr/bin/env bash
# Compila el launcher Linux (x86_64 nativo).
# Requiere: gcc, pkg-config, libsdl2-dev
set -euo pipefail
cd "$(dirname "$0")"

cc -O2 -std=gnu17 launcher_linux.c -o launcher_linux \
   $(pkg-config --cflags --libs sdl2) -lm

echo "OK: launcher/launcher_linux"
