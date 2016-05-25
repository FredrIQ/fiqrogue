/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "level.h"
#include "rogue.h"
#include "vision.h"

/* Field of vision handling, lightning, monster senses, etc */

static void fov_recalc_line(int, int);
static void fov_reset(void);
static void fov_mark_explored(void);

/* Calculates vision of everything. Used for player vision. Monster vision is
   instead checked on-demand (ultimately needs far less calculations).
   The algoritm is: for each applicable edge, draw a line from the player to it,
   stopping at obstacles, and flip the vision bit for those */
void
fov_recalc(void)
{
    fov_reset();

    int x, y;

    for (x = 0; x < ROOMSIZEX; x++) {
        for (y = 0; y < ROOMSIZEY; y++) {
            /* Only calculate edges */
            if (x > 0 && x < ROOMSIZEX - 1 &&
                y > 0 && y < ROOMSIZEY - 1)
                continue;

            fov_recalc_line(x, y);
        }
    }

    fov_mark_explored();
}

bool
has_obstacle(struct level *level, int x, int y) {
    if ((level->prop[x][y] & MAP_TYPEMASK) <= MAP_LAST_SOLID)
        return true;
    return false;
}

/* Recalculates FOV for one line using Bresemham's algoritm */
static void
fov_recalc_line(int x1, int y1)
{
    struct level *level = pmon.level;
    int x = pmon.x;
    int y = pmon.y;
    int dx = abs(x1 - x), sx = x < x1 ? 1 : -1;
    int dy = -abs(y1 - y), sy = y < y1 ? 1 : -1;
    int err = dx + dy, e2;
    bool obstructed = false;

    while (true) {
        if (!obstructed) {
            level->prop[x][y] |= MAP_VISIBLE;
            if (has_obstacle(level, x, y))
                obstructed = true;
        } else if (!has_obstacle(level, x, y))
            level->prop[x][y] &= ~MAP_VISIBLE;

        if (x == x1 && y == y1)
            break;

        e2 = 2 * err;
        if (e2 > dy) {
            err += dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

/* Clears visibility everywhere */
static void
fov_reset(void)
{
    struct level *level = pmon.level;
    int x, y;
    for (x = 0; x < ROOMSIZEX; x++)
        for (y = 0; y < ROOMSIZEY; y++)
            level->prop[x][y] &= ~MAP_VISIBLE;
}

static void
fov_mark_explored(void)
{
    struct level *level = pmon.level;
    int x, y;
    for (x = 0; x < ROOMSIZEX; x++)
        for (y = 0; y < ROOMSIZEY; y++)
            if ((level->prop[x][y] & MAP_VISIBLE))
                level->prop[x][y] |= MAP_EXPLORED;
}
