#!/usr/bin/env bash
# Cross-compila el launcher para Raspberry Pi (ARM64 / aarch64) desde un PC x86_64.
#
# Requisitos (Ubuntu/Debian):
#   sudo dpkg --add-architecture arm64
#   # añade un repo arm64 (ports.ubuntu.com o deb.debian.org) y:
#   sudo apt install gcc-aarch64-linux-gnu libsdl2-dev:arm64 pkg-config
#
# Uso:
#   ./build-pi.sh
#   PKG_CONFIG_LIBDIR=/ruta/arm64/pkgconfig ./build-pi.sh
set -euo pipefail
cd "$(dirname "$0")"

CROSS="${CROSS:-aarch64-linux-gnu-}"
export PKG_CONFIG_LIBDIR="${PKG_CONFIG_LIBDIR:-/usr/lib/aarch64-linux-gnu/pkgconfig}"

"${CROSS}gcc" -O2 -std=gnu17 launcher_linux.c -o launcher_arm64 \
   $(pkg-config --cflags --libs sdl2) -lm

echo "OK: launcher/launcher_arm64 (aarch64)"
