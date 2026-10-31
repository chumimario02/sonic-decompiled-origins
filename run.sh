#!/usr/bin/env bash
# Abre el menu de Sonic Decompiled Origins.
set -euo pipefail
cd "$(dirname "$0")"

if [ ! -x launcher/launcher ]; then
  echo "Falta el lanzador. Ejecuta: ./launcher/build.sh"
  exit 1
fi

exec ./launcher/launcher