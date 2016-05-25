/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "level.h"
#include "rogue.h"

/* Level management */

struct level *
level_init(int z)
{
    struct level *level = malloc(sizeof (struct level));
    memset(level, 0, sizeof (struct level));
    int x, y;

    /* Make all of the level solid */
    for (x = 0; x < ROOMSIZEX; x++)
        for (y = 0; y < ROOMSIZEY; y++)
            level->prop[x][y] |= MAP_ROCK;

    level->z = z;

    int i;
    for (i = 0; i < NUM_LEVELS; i++) {
        if (!gamestate.levels[i]) {
            gamestate.levels[i] = level;
            break;
        }
    }

    return level;
}
