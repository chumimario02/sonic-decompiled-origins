#!/usr/bin/env bash
# Clona los cuatro proyectos de recompilación y su motor compartido.
set -euo pipefail

cd "$(dirname "$0")/.."

REPOS=(
  SonicTheHedgehogRecomp
  SonicTheHedgehog2Recomp
  Sonic3AndKnucklesRecomp
  SonicTheHedgehogSMSRecomp
)

mkdir -p src
cd src

for repo in "${REPOS[@]}"; do
  if [ -d "$repo" ]; then
    echo "== $repo (ya existe, actualizando)"
    git -C "$repo" pull --ff-only || true
  else
    echo "== clonando $repo"
    git clone --recursive "https://github.com/mstan/$repo.git"
  fi
  # Algunos repos necesitan el enlace al motor compartido.
  if [ -x "$repo/scripts/link-engine.sh" ]; then
    ( cd "$repo" && ./scripts/link-engine.sh ) || true
  fi
done

echo
echo "Listo. Ahora: ./scripts/build-pi.sh"
