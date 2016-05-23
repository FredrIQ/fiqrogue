/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "mon.h"
#include "obj.h"
#include "rogue.h"
#include "ui.h"

/* Monster handling */

/*
 * Monster action.
 * Returns:
 * ACT_FREE - can act further this turn
 * ACT_DONE - is done for the turn
 * ACT_DIED - died
 */
enum act
mon_act(struct mon *mon)
{
    struct obj *obj;
    enum cmd cmd = CMD_NONE;
    if (mon == &pmon) {
        /* Refresh the UI before input */
        ui_refresh();
        do {
            cmd = ui_cmd();
        } while (cmd == CMD_NONE);
    } else
        cmd = (mon->y == 30 ? CMD_LEFT : CMD_RIGHT); /* dummy AI */

    switch (cmd) {
    case CMD_HELP:
        pline("Some quick help: h=Left, j=Down, k=Up, l=Right, ,=Pickup");
        return ACT_FREE;
    case CMD_QUIT:
        return ACT_DIED;
    case CMD_PICKUP:
        obj = obj_at(mon->x, mon->y);
        if (!obj) {
            pline("Nothing to pickup!");
            return ACT_FREE; /* nothing here, don't waste a turn */
        }

        pickobj(mon, obj);
        return ACT_DONE;
    case CMD_LEFT:
    case CMD_DOWN:
    case CMD_UP:
    case CMD_RIGHT:
        if (cmd == CMD_LEFT)
            mon->y--;
        else if (cmd == CMD_RIGHT)
            mon->y++;
        else if (cmd == CMD_UP)
            mon->x--;
        else if (cmd == CMD_DOWN)
            mon->x++;
        if (mon->x < 0)
            mon->x = 0;
        if (mon->x == ROOMSIZEX)
            mon->x--;
        if (mon->y < 0)
            mon->y = 0;
        if (mon->y == ROOMSIZEY)
            mon->y--;
        break;
    case CMD_UNKNOWN:
        pline("Unknown command!");
        return ACT_FREE;
    default:
        pline("Unhandled case: %d", cmd);
        return ACT_FREE;
    }

    return ACT_DONE;
}

/* Deallocate a monster */
void
mon_free(struct mon *mon)
{
    if (!mon)
        return;
    monlist_free(mon);
    free(mon);
}

/* Create a new monster */
struct mon *
mon_new(enum montyp typ, int x, int y, bool add_to_monlist)
{
    struct mon *mon = malloc(sizeof (struct mon));
    memset(mon, 0, sizeof (struct mon));
    if (x >= ROOMSIZEX)
        x = ROOMSIZEX - 1;
    if (y >= ROOMSIZEY)
        y = ROOMSIZEY - 1;

    mon->x = x;
    mon->y = y;
    if (typ > LAST_MON)
        typ = LAST_MON;
    mon->typ = typ;

    /* Add to the monster list */
    if (add_to_monlist)
        monlist_add(mon);
    return mon;
}

/* Add a monster to the list */
bool
monlist_add(struct mon *mon)
{
    if (!mon)
        return false; /* shouldn't happen */

    struct mon *mtmp;
    for (mtmp = gamestate.monlist; mtmp; mtmp = mtmp->nmon) {
        if (mon == mtmp)
            return false; /* already in the list */
    }

    mon->nmon = gamestate.monlist;
    gamestate.monlist = mon;
    return true;
}

/* Free a monster from the list */
bool
monlist_free(struct mon *mon)
{
    if (!mon)
        return false; /* oops? */

    struct mon *prevmon = NULL;
    struct mon *listmon = NULL;
    for (listmon = gamestate.monlist; listmon; listmon = listmon->nmon) {
        if (mon != listmon) {
            prevmon = listmon;
            continue;
        }

        if (prevmon)
            prevmon->nmon = listmon->nmon;
        else /* listmon is gamestate.monlist */
            gamestate.monlist = listmon->nmon;
        return true; /* we're done here */
    }
    return false; /* not in the list */
}
