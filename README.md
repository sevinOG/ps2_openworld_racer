# PS2 Open-World Racer

Homebrew PlayStation 2 game built with [Tyra](https://github.com/h4570/tyra) and the [ps2dev](https://github.com/ps2dev/ps2dev) toolchain.

Target: **real PS2 hardware architecture** (Emotion Engine / VU0 / VU1 / GS / IOP). Not a PC port.

## Requirements

- Windows 10/11 + WSL2 Ubuntu 24.04
- Docker Engine **inside WSL** (not required on Windows)
- Project lives on the Linux filesystem: `~/projects/ps2-openworld-racer`

## Build

From WSL:

```bash
cd ~/projects/ps2-openworld-racer
./scripts/build.sh
```

Output: `bin/racer.elf`

Clean:

```bash
./scripts/clean.sh
```

## Layout

```
src/            Game C++ sources
inc/            Game headers
res/            Runtime assets copied next to the ELF
scripts/        Agent-friendly Docker wrappers
bin/            Built ELF
vendor/tyra/    Tyra engine (git submodule)
```

See `AGENTS.md` for agent/build details.
