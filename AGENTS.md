# AGENTS.md — PS2 Open-World Racer

## Target

Pure PlayStation 2 architecture. Code compiles with the ps2dev EE toolchain (`mips64r5900el-ps2-elf-g++`) and runs as a `.elf` on PCSX2 or real hardware. Do not introduce PC-only APIs, SDL, OpenGL, or host threading.

## How to build

Always compile **inside WSL + Docker**. Never compile on the Windows filesystem (`/mnt/c/...`).

```bash
cd ~/projects/ps2-openworld-racer
./scripts/build.sh
```

Equivalent raw command:

```bash
docker run --rm -v "$(pwd)":/src -w /src h4570/tyra \
  bash -lc 'make build-engine && make'
```

Clean:

```bash
./scripts/clean.sh
```

We use the official **`h4570/tyra`** image for game/engine builds because Tyra's VU1 programs need Sony `vcl` + `vclpp` (not present in `ps2dev/ps2dev` alone). The `ps2dev/ps2dev:latest` image is installed and verified for raw toolchain work (`mips64r5900el-ps2-elf-gcc`).

There is no `ee-gcc` binary in current ps2dev images. The EE compiler is `mips64r5900el-ps2-elf-gcc`.

## Project layout

| Path | Purpose |
|------|---------|
| `src/` | Game sources (`main.cpp`, `racer_game.cpp`) |
| `inc/` | Game headers |
| `res/` | Assets copied to `bin/` at build time |
| `scripts/build.sh` | Dockerized `make` (builds engine if needed) |
| `scripts/clean.sh` | Dockerized `make cleaner` + engine clean |
| `scripts/run-pcsx2.sh` | Placeholder until PCSX2 is installed on Windows |
| `bin/` | Output `racer.elf` |
| `vendor/tyra/` | Tyra engine submodule |
| `Makefile` | Tyra template Makefile; `ENGINEDIR := vendor/tyra/engine` |

## Rules for agents

- Work in `~/projects/ps2-openworld-racer` (Linux ext4), not `/mnt/c/...`.
- All compilation via Docker (`h4570/tyra` for Tyra, `ps2dev/ps2dev` for raw toolchain).
- Keep the game on Tyra's template structure (`src` / `inc` / `res` / `bin` / `Makefile` + `Makefile.base`).
- Commit clean states frequently.
- Do not vendor large binary assets without asking.

## Current status

See the bottom of this file; update it when the bootstrap state changes.

### Bootstrap (filled in after verification)

- Phase 0: WSL2 Ubuntu 24.04 ready
- Phase 1: Docker Engine inside WSL
- Phase 2: `ps2dev/ps2dev:latest` pulled; EE gcc works
- Phase 3: this repo created
- Phase 4: Tyra cloned; sample build pending
- Phase 5: `scripts/build.sh` and `scripts/clean.sh` present
- Phase 6: this file
