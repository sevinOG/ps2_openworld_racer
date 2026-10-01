#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
docker run --rm --user "$(id -u):$(id -g)" -e HOME=/tmp \
  -v "$(pwd)":/src -w /src h4570/tyra make cleaner || true
