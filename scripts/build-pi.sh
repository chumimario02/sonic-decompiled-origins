#!/usr/bin/env bash
# Compila los cuatro juegos en la Raspberry Pi y copia los binarios a games/.
set -euo pipefail

cd "$(dirname "$0")/.."
ROOT="$(pwd)"
mkdir -p games

# repo|nombre del binario resultante|carpeta de salida
GAMES=(
  "SonicTheHedgehogRecomp|sonic1|sonic1"
  "SonicTheHedgehog2Recomp|sonic2|sonic2"
  "Sonic3AndKnucklesRecomp|s3k|s3k"
  "SonicTheHedgehogSMSRecomp|sonicsms|sonicsms"
)

for entry in "${GAMES[@]}"; do
  IFS='|' read -r repo bin out <<< "$entry"

  if [ ! -d "src/$repo" ]; then
    echo "!! Falta src/$repo. Ejecuta antes ./scripts/fetch.sh"
    exit 1
  fi

  echo
  echo "=============================================================="
  echo "== Compilando $repo"
  echo "=============================================================="

  pushd "src/$repo" >/dev/null

  if [ -x tools/build-linux.sh ]; then
    ./tools/build-linux.sh --config prod || ./tools/build-linux.sh
  else
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build
  fi

  # Buscar el ejecutable recién compilado (los layouts cambian entre repos).
  found="$(find . -type f -perm -u+x -name "$bin" -o -type f -perm -u+x -name "${bin}*" 2>/dev/null | grep -v -E '\.(o|a|so|c|cpp|h)$' | head -n 1 || true)"

  if [ -z "$found" ]; then
    echo "!! No encontré el binario '$bin' en src/$repo"
    echo "   Revisa la salida de la compilación y ajusta la ruta a mano."
    popd >/dev/null
    continue
  fi

  cp "$found" "$ROOT/games/$out"
  chmod +x "$ROOT/games/$out"
  echo "== OK: games/$out"
  popd >/dev/null
done

echo
echo "Binarios en games/:"
ls -1 games || true
echo
echo "Recuerda poner tus ROMs en roms/ y luego ejecutar ./run.sh"
