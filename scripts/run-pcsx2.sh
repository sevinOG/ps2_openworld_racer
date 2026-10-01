#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
EXE="/mnt/c/Program Files/PCSX2/pcsx2-qt.exe"
GAMES="/mnt/c/Users/ananb/Documents/PCSX2/games"
ELF="$GAMES/racer.elf"
if [ -f bin/racer.elf ]; then
  mkdir -p "$GAMES"
  cp -f bin/racer.elf "$ELF"
fi
if [ ! -f "$ELF" ]; then
  echo "No ELF. Run ./scripts/build.sh first."
  exit 1
fi
cmd.exe /c start "" "$(wslpath -w "$EXE")" -fastboot -elf "$(wslpath -w "$ELF")"
echo "PCSX2 launched: $(wslpath -w "$ELF")"
