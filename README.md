# PS2 Open-World Racer

Homebrew PlayStation 2 game built with [Tyra](https://github.com/h4570/tyra).
A penguin walks a snowy mountain map, drives an off-road buggy, and draws control arms at a wax bench.

## Build

From WSL:

```bash
cd /home/sevin/projects/ps2-openworld-racer
./scripts/build.sh
```

That runs `make` in the `h4570/tyra` Docker image as your user, writes `bin/racer.elf`, and copies it to the PCSX2 games folder. Quit PCSX2 fully before booting the new ELF.

The Tyra checkout is stock. `./scripts/build.sh` applies `patches/tyra-line-batch.patch` when the line batcher is not already there, and it rebuilds `vendor/tyra/engine/bin/libtyra.a` only if that archive is missing.

From Windows, `C:\Users\ananb\ps2-racer\build.cmd` calls the same script. Do not run `docker` or `make` from Windows.

## Controls

| Input | On foot | In the car | At the bench |
|-------|---------|------------|--------------|
| Left stick | Walk | Steer | Move the draw cursor |
| Right stick | Look | Look | Orbit the table |
| R2 / L2 | | Gas / brake | |
| Cross | Jump | | |
| Triangle | Enter the car or the bench | Exit | Exit |
| Square | | Snap a part on | Hold and trace the arm |
| Circle | | Weld (1 wax) | Copy the part (2 wax) |
| L1 | | Torch a weld | Put a pack part on the table |
| R1 | | Cycle the corner | |
| Select | | | Hide or show the instructions |
| D-pad | | | Up/down picks the arm, left/right sets the width |

## Layout

```
src/  inc/       Game code
res/             Runtime assets
scripts/         Docker build, clean, permissions, PCSX2 launch
patches/         Tyra fixes applied by build.sh
vendor/tyra/     Tyra engine (git submodule)
tutorial.txt     Build rules for people and agents
```
