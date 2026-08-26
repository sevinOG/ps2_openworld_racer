# AGENTS.md — PS2 Open-World Racer

## Target

Pure PlayStation 2 architecture. Code compiles with the ps2dev EE toolchain
(`mips64r5900el-ps2-elf-g++`) and runs as a `.elf` on PCSX2 or real hardware.
Do not introduce PC-only APIs, SDL, OpenGL, or host threading.

## How to build

Always compile **inside WSL + Docker**. Never compile on the Windows filesystem
(`/mnt/c/...`). Project path:

```
/home/sevin/projects/ps2-openworld-racer
```

Windows Explorer: `\\wsl$\Ubuntu\home\sevin\projects\ps2-openworld-racer`

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

Output ELF: `bin/racer.elf`

### Which Docker image

| Image | Use |
|-------|-----|
| `h4570/tyra` | Game + Tyra engine builds (has Sony `vcl` + `vclpp` for VU1) |
| `ps2dev/ps2dev:latest` | Raw toolchain checks (`mips64r5900el-ps2-elf-gcc`) |

There is **no `ee-gcc`** binary in current ps2dev images. The EE compiler is
`mips64r5900el-ps2-elf-gcc` (GCC 15.2.0 in `ps2dev/ps2dev`, GCC 11.3.0 in
`h4570/tyra`).

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
| `vendor/tyra/` | Tyra engine git submodule |
| `Makefile` | Tyra template Makefile; `ENGINEDIR := vendor/tyra/engine` |

## Rules for agents

- Work in `~/projects/ps2-openworld-racer` (Linux ext4), not `/mnt/c/...`.
- All compilation via Docker (`h4570/tyra` for Tyra, `ps2dev/ps2dev` for raw toolchain).
- Keep the game on Tyra's template structure (`src` / `inc` / `res` / `bin` / `Makefile`).
- Commit clean states frequently. Do not commit `.elf`, `.o`, or `libtyra.a`.
- Do not vendor large binary assets without asking.
- After `wsl --shutdown`, Docker needs the distro started again (`wsl -d Ubuntu`).

## Current status (2026-08-25)

- **Phase 0** — WSL2 Ubuntu 24.04.4 LTS, user `sevin`. systemd enabled.
- **Phase 1** — Docker Engine 29.7.2 inside WSL. `hello-world` runs without sudo. User is in the `docker` group.
- **Phase 2** — `ps2dev/ps2dev:latest` pulled. `mips64r5900el-ps2-elf-gcc (GCC) 15.2.0` works. (`ee-gcc` does not exist; name changed.)
- **Phase 3** — Repo created at `~/projects/ps2-openworld-racer`.
- **Phase 4** — Tyra cloned as `vendor/tyra` (submodule, `h4570/tyra`). Official image `h4570/tyra` pulled. Engine `libtyra.a` built. Official sample `tutorials/01-hello` built as `tutorial_01.elf` (ELF 32-bit LSB, MIPS N32). Game scaffolded from Tyra's racer template. `bin/racer.elf` built (same MIPS ELF).
- **Phase 5** — `scripts/build.sh`, `scripts/clean.sh`, placeholder `scripts/run-pcsx2.sh`.
- **Phase 6** — this file.

### Not done yet

- PCSX2 on Windows (needed to actually run the ELF).
- Gameplay beyond Tyra's hello-racer template (`TYRA_LOG` in `init()`).
