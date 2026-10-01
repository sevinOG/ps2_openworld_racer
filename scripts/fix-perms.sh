#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
uid="$(id -u)"
gid="$(id -g)"
mkdir -p bin obj
docker run --rm --user 0 -v "$(pwd)":/src -w /src h4570/tyra \
  chown -R "${uid}:${gid}" bin obj vendor/tyra/engine/bin vendor/tyra/engine/obj
echo "bin/ and obj/ now owned by ${uid}:${gid}"
