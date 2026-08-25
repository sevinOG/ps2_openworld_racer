#!/usr/bin/env bash
# Remove game and engine build artifacts.
set -euo pipefail
cd "$(dirname "$0")/.."

IMAGE="${TYRA_IMAGE:-h4570/tyra}"

if docker image inspect "$IMAGE" >/dev/null 2>&1; then
  docker run --rm \
    -v "$(pwd)":/src \
    -w /src \
    "$IMAGE" \
    bash -lc 'make cleaner || true; make clean-engine || true'
else
  rm -rf obj bin/*.elf vendor/tyra/engine/obj vendor/tyra/engine/bin/*.a
fi

echo "Clean complete."
