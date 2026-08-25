#!/usr/bin/env bash
# Placeholder: PCSX2 lives on the Windows side.
# After installing PCSX2, point PCSX2_EXE at pcsx2-qt.exe and this will
# launch the built ELF. Example:
#   PCSX2_EXE="/mnt/c/Program Files/PCSX2/pcsx2-qt.exe" ./scripts/run-pcsx2.sh
set -euo pipefail
cd "$(dirname "$0")/.."

ELF="$(pwd)/bin/racer.elf"
if [[ ! -f "$ELF" ]]; then
  echo "No ELF yet. Run ./scripts/build.sh first."
  exit 1
fi

if [[ -z "${PCSX2_EXE:-}" ]]; then
  echo "PCSX2 is not installed/configured yet (Windows-side)."
  echo "Built ELF: $ELF"
  echo "Later: set PCSX2_EXE to your pcsx2-qt.exe path, then re-run this script."
  exit 0
fi

# Convert Linux path to a Windows path PCSX2 can open.
WIN_ELF="$(wslpath -w "$ELF")"
exec "$PCSX2_EXE" -elf "$WIN_ELF"
