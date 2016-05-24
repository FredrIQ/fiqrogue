/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "mon.h"
#include "obj.h"
#include "rogue.h"
#include "ui.h"

/* Monster handling */

static bool mon_move(struct mon *, struct command *);

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
    struct command cmd = {0};
    cmd.typ = CMD_NONE;

    if (mon == &pmon) {
        /* Refresh the UI before input */
        do {
            ui_refresh();
            ui_cmd(&cmd);
        } while (cmd.typ == CMD_NONE);
    } else {
        cmd.typ = CMD_MOVE;
        cmd.dx = (mon->x == 10 ? -1 : 1); /* dummy AI */
    }

    switch (cmd.typ) {
    case CMD_HELP:
        pline("Some quick help: h=Left, j=Down, k=Up, l=Right, ,=Pickup, "
              "S=quit.");
        pline("You can also use the numpad or arrow keys to move.");
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
    case CMD_MOVE:
        mon_move(mon, &cmd);
        return ACT_DONE;
    case CMD_UNKNOWN:
        pline("Unknown command!");
        return ACT_FREE;
    default:
        pline("Unhandled case: %d", cmd);
        return ACT_FREE;
    }

    return ACT_DONE;
}

/* Returns the monster at the given location, or NULL if there's nothing
   there */
struct mon *
mon_at(int x, int y)
{
    struct mon *mon;
    for (mon = gamestate.monlist; mon; mon = mon->nmon)
    {
        if (mon->dead)
            continue;

        if (mon->x == x && mon->y == y)
            return mon;
    }

    return NULL;
}

/* Movement handling */
static bool
mon_move(struct mon *mon, struct command *cmd)
{
    if (!mon)
        return false;

    bool you = (mon == &pmon);
    int x = mon->x, y = mon->y;
    x += cmd->dx;
    y += cmd->dy;

    if (x < 0 || x >= ROOMSIZEX || y < 0 || y >= ROOMSIZEY) {
        if (you)
            pline("That's a wall.");
        return false;
    }

    if (mon_at(x, y)) {
        if (you)
            pline("Sorry, you can't fight stuff yet.");
        return false;
    }

    mon->x = x;
    mon->y = y;
    return true;
}
