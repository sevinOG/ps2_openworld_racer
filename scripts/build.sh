#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
IMAGE="${TYRA_IMAGE:-h4570/tyra}"
mkdir -p bin obj
# Upstream Tyra has no line batcher, and its dep rule calls fmt, which the
# h4570/tyra image does not ship. Apply our patch onto a clean submodule.
if ! grep -q 'void pushLine' vendor/tyra/engine/inc/renderer/3d/renderer_3d_utility.hpp; then
  git -C vendor/tyra apply --whitespace=nowarn "$PWD/patches/tyra-line-batch.patch"
fi
docker run --rm --user "$(id -u):$(id -g)" -e HOME=/tmp \
  -v "$(pwd)":/src -w /src "$IMAGE" \
  bash -lc 'set -e
    if [ ! -f vendor/tyra/engine/bin/libtyra.a ]; then
      echo "==> engine"
      make build-engine
    fi
    echo "==> game"
    make
  '
ls -lh bin/racer.elf
GAMES="/mnt/c/Users/ananb/Documents/PCSX2/games"
if [ -d /mnt/c/Users/ananb/Documents/PCSX2 ]; then
  mkdir -p "$GAMES"
  cp -f bin/racer.elf "$GAMES/racer.elf"
  echo "copied to $GAMES/racer.elf"
fi
