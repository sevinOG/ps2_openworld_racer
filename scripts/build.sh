#!/usr/bin/env bash
# Build the game (and Tyra engine if needed) inside the official Tyra image.
# The Tyra image is required because VU1 programs need Sony VCL + vclpp.
set -euo pipefail
cd "$(dirname "$0")/.."

IMAGE="${TYRA_IMAGE:-h4570/tyra}"

if [[ ! -d vendor/tyra/engine ]]; then
  echo "Tyra is missing. Clone it first:"
  echo "  git submodule update --init --recursive"
  exit 1
fi

docker run --rm \
  -v "$(pwd)":/src \
  -w /src \
  "$IMAGE" \
  bash -lc 'set -euo pipefail
    if [[ ! -f vendor/tyra/engine/bin/libtyra.a ]]; then
      echo "==> Building Tyra engine"
      make build-engine
    fi
    echo "==> Building racer.elf"
    make
  '

echo "ELF: $(pwd)/bin/racer.elf"
ls -lh bin/racer.elf 2>/dev/null || true
