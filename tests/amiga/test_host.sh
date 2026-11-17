#!/bin/sh
# Pacman3D for GFOS, host tests: the six datafiles and the 12 levels read
# back by gfopenallegro's own code (fake acgame.library).
#   tests/amiga/test_host.sh A4_DIR     (gfopenallegro/allegro4)
# Copyright (c) 2026 AmiCradle Team. MIT, as Pacman3D.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
A4=${1:?gfopenallegro/allegro4 directory}
T=${TMPDIR:-/tmp}/pac3d-host-$$
mkdir -p "$T"
trap 'rm -rf "$T"' EXIT HUP INT TERM
CC=${HOSTCC:-cc}
SRCS=$(ls "$A4"/src/a4_*.c | grep -v -e a4_os_amiga -e a4_amiga -e a4_dumb_gm)
FAKES="$A4/tests/fake_acgame.c"
if [ -f "$A4/tests/fake_music.c" ]; then FAKES="$FAKES $A4/tests/fake_music.c"; fi
"$CC" -std=gnu99 -D_DEFAULT_SOURCE -w -O1 -I"$ROOT" -I"$A4/include" -I"$A4/src" -I"$A4/tests" \
  $SRCS $FAKES "$ROOT/tests/amiga/test_data.c" -o "$T/test_data" -lm
"$T/test_data" "$ROOT"
echo "host tests: PASS"
