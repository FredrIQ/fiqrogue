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
    int x, y, i;

    level->z = z;

    for (i = 0; i < NUM_LEVELS; i++) {
        if (!gamestate.levels[i]) {
            gamestate.levels[i] = level;
            break;
        }
    }

    /* Make all of the level solid */
    for (x = 0; x < ROOMSIZEX; x++)
        for (y = 0; y < ROOMSIZEY; y++)
            level->prop[x][y] |= MAP_ROCK;

    /* Create rooms */
    int room;
    int rooms = rn2(5) + 3;
    int roomtop[8];
    int roomleft[8];
    int roomheight[8];
    int roomwidth[8];

    /* Create the 1st room somewhere */
    for (room = 0; room < rooms; room++) {
        roomheight[room] = rn2(8) + 6;
        roomwidth[room] = rn2(8) + 6;
        roomtop[room] = rn2(ROOMSIZEY - roomheight[room]);
        roomleft[room] = rn2(ROOMSIZEX - roomwidth[room]);

        int xoffset = roomleft[room];
        int yoffset = roomtop[room];
        for (x = 0; x < roomwidth[room]; x++) {
            for (y = 0; y < roomheight[room]; y++) {
                if ((x == 0 || x == roomwidth[room] - 1 ||
                     y == 0 || y == roomheight[room] - 1) &&
                    level->prop[x + xoffset][y + yoffset] == MAP_ROCK)
                    level->prop[x + xoffset][y + yoffset] = MAP_XWALL;
                else {
                    level->prop[x + xoffset][y + yoffset] = MAP_FLOOR;
                    level->prop[x + xoffset][y + yoffset] |= MAP_LIT;
                }
            }
        }
        if (!room)
            continue; /* First room can't connect anywhere... */

        //continue;
        int xdir, ydir, xgoal, ygoal;
        x = rn2(roomwidth[room]-2) + roomleft[room] + 1;
        y = rn2(roomheight[room]-2) + roomtop[room] + 1;
        xgoal = rnd(roomwidth[room-1]-2) + roomleft[room-1] + 1;
        ygoal = rnd(roomheight[room-1]-2) + roomtop[room-1] + 1;
        if (x < xgoal)
            xdir = 1;
        else
            xdir = -1;
        if (y < ygoal)
            ydir = 1;
        else
            ydir = -1;
        while (x != xgoal) {
            x += xdir;
            if (level->prop[x][y] <= MAP_LAST_WALL) {
                level->prop[x][y] = MAP_FLOOR;
                level->prop[x][y] |= MAP_LIT;
            } else if (level->prop[x][y] <= MAP_LAST_SOLID)
                level->prop[x][y] = MAP_CORRIDOR;
        }
        while (y != ygoal) {
            y += ydir;
            if (level->prop[x][y] <= MAP_LAST_WALL) {
                level->prop[x][y] = MAP_FLOOR;
                level->prop[x][y] |= MAP_LIT;
            } else if (level->prop[x][y] <= MAP_LAST_SOLID)
                level->prop[x][y] = MAP_CORRIDOR;
        }
    }
    return level;
}
