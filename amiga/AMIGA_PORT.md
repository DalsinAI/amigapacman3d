# Pacman3D for GFOS

Pacman3D 1.4 by Andrew Pike (fifsoft; MIT, see ReadMe-Pacman3D), built
for AmigaOS 3.2 / GFOS on a 68040, 68060 or 68080. No FPU is needed.

A maze chase in an isometric 3D view: eat every dot across 12 levels
while four ghosts hunt you, with power pills, bonus objects and a
save-game.

## What is in the drawer

- `Pacman3D`: the game (320x200, 256 colours).
- `data/`: its six Allegro datafiles and the 12 levels.
- `pacman.ini`: extra settings. `hiscore.dat` and `save.dat` are made by
  the game. Updates keep all three.
- `Libs/allegro4.library` and `Libs/acgame.library`: Allegro 4 for GFOS
  (gfopenallegro) on ACGame. The game opens them from its own `Libs/`.

## How the port is made

- The code is upstream's, unchanged, built with 68040 soft float against
  gfopenallegro's Allegro 4 (`amiga/Makefile.amigaos`).
- The game opens its files relative to the current drawer, so it moves
  into its own drawer (`PROGDIR:`) when it starts (`amiga/amiga_start.c`).
- The release's `hiscore.dat` and `save.dat` are raw x86 structures, so
  they are not shipped; the game makes fresh ones in the Amiga's byte
  order.

## Tests

`tests/amiga/test_host.sh` reads the six datafiles through
gfopenallegro's own code, checks them against the indexes the game uses
(`data/pacdata.h`, `data/enem.h`), and checks the 12 levels the way the
game reads them. `package/build-amiga.sh` runs it before each build.
