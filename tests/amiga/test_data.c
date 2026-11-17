/* Pacman3D for GFOS: the game's six datafiles read back through
 * gfopenallegro's own code (host build, fake acgame.library), checked
 * against the indexes pacman.c uses (data/pacdata.h, data/enem.h), and
 * the 12 levels checked the way pac_ext.c's load_level() reads them.
 *   test_data GAME_DIR
 * Copyright (c) 2026 AmiCradle Team. MIT, as Pacman3D. */
#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "data/pacdata.h"
#include "data/enem.h"

static int fails, checks;
#define CHECK(c) do { checks++; if (!(c)) { fails++; printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); } } while (0)

static int count(DATAFILE *d)
{
    int n = 0;
    while (d && d[n].type != DAT_END) n++;
    return n;
}

static DATAFILE *load(const char *dir, const char *name, int min)
{
    char path[1024];
    DATAFILE *d;
    int i, n;
    snprintf(path, sizeof(path), "%s/data/%s", dir, name);
    d = load_datafile(path);
    CHECK(d != NULL);
    if (!d) { printf("  cannot load %s\n", name); return NULL; }
    n = count(d);
    CHECK(n >= min);
    for (i = 0; i < n; i++) CHECK(d[i].dat != NULL);
    printf("%s: %d objects\n", name, n);
    return d;
}

int main(int argc, char **argv)
{
    char path[1024];
    DATAFILE *d;
    int lev, ok = 0;

    if (argc < 2) return 2;
    CHECK(allegro_init() == 0);
    set_color_depth(8);
    d = load(argv[1], "pacdata.dat", YOUWIN + 1);
    if (d) {
        CHECK(d[YOULOSE].type == DAT_BITMAP && d[YOUWIN].type == DAT_BITMAP);
        unload_datafile(d);
    }
    d = load(argv[1], "enem.dat", GHOST1 + 1);
    if (d) { CHECK(d[GHOST0].type == DAT_BITMAP); unload_datafile(d); }
    d = load(argv[1], "mwf.dat", 1);  if (d) unload_datafile(d);
    d = load(argv[1], "bg.dat", 1);   if (d) { CHECK(d[0].type == DAT_BITMAP); unload_datafile(d); }
    d = load(argv[1], "fl.dat", 1);   if (d) { CHECK(d[0].type == DAT_BITMAP); unload_datafile(d); }
    d = load(argv[1], "obj.dat", 1);  if (d) { CHECK(d[0].type == DAT_BITMAP); unload_datafile(d); }

    for (lev = 1; lev <= 12; lev++) {   /* load_level: "rp", start x/y, 50 x 100 cells */
        FILE *f;
        int c, n = 0;
        snprintf(path, sizeof(path), "%s/data/lev%d.pac", argv[1], lev);
        f = fopen(path, "rb");
        CHECK(f != NULL);
        if (!f) continue;
        CHECK(fgetc(f) == 114 && fgetc(f) == 'p');
        fgetc(f); fgetc(f);
        while ((c = fgetc(f)) != EOF && n < 5000) n++;
        CHECK(n == 5000);
        fclose(f);
        ok++;
    }
    CHECK(ok == 12);
    printf("%d levels; %d checks, %d failed\n", ok, checks, fails);
    return fails ? 1 : 0;
}
