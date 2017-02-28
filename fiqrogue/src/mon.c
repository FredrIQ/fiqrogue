/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "mon.h"
#include "obj.h"
#include "random.h"
#include "rogue.h"
#include "ui.h"
#include "vision.h"

/* Monster handling */

static bool mon_move(struct mon *, struct command *);

/* Deallocate a monster */
void
mon_free(struct mon *mon)
{
    if (!mon)
        return;
    monlist_free(mon);
    if (mon == &pmon) {
        pline("Trying to free the player monst???");
    } else
        free(mon);
}

/* Create a new monster. level is which level to add the monster to, or NULL to not
   add it anywhere. */
struct mon *
mon_new(struct level *level, enum montyp typ, int x, int y)
{
    struct mon *mon = malloc(sizeof (struct mon));
    memset(mon, 0, sizeof (struct mon));
    if (x >= ROOMSIZEX || y >= ROOMSIZEY) {
        /* Find a nonsolid place to place the monster */
        do {
            x = rn2(ROOMSIZEX);
            y = rn2(ROOMSIZEY);
        } while (has_obstacle(level, x, y));
    }

    mon->x = x;
    mon->y = y;
    if (typ > LAST_MON)
        typ = LAST_MON;
    mon->typ = typ;
    mon->level = level;
    mon->hp = 1;

    /* Add to the monster list */
    if (level)
        monlist_add(level, mon);
    return mon;
}

/* Add a monster to the list */
bool
monlist_add(struct level *level, struct mon *mon)
{
    if (!mon)
        return false; /* shouldn't happen */

    struct mon *mtmp;
    for (mtmp = level->monlist; mtmp; mtmp = mtmp->nmon) {
        if (mon == mtmp)
            return false; /* already in the list */
    }

    mon->nmon = level->monlist;
    level->monlist = mon;
    return true;
}

/* Free a monster from the list */
bool
monlist_free(struct mon *mon)
{
    if (!mon)
        return false; /* oops? */

    struct level *level = mon->level;
    struct mon *prevmon = NULL;
    struct mon *listmon = NULL;
    for (listmon = level->monlist; listmon; listmon = listmon->nmon) {
        if (mon != listmon) {
            prevmon = listmon;
            continue;
        }

        if (prevmon)
            prevmon->nmon = listmon->nmon;
        else /* listmon is gamestate.monlist */
            level->monlist = listmon->nmon;
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
        if (mon->x < pmon.x &&
            !has_obstacle(mon->level, mon->x + 1, mon->y))
            cmd.dx = 1;
        else if (mon->x > pmon.x &&
                 !has_obstacle(mon->level, mon->x - 1, mon->y))
            cmd.dx = -1;

        if (mon->y < pmon.y &&
                 !has_obstacle(mon->level, mon->x, mon->y + 1))
            cmd.dy = 1;
        else if (mon->y > pmon.y &&
                 !has_obstacle(mon->level, mon->x, mon->y - 1))
            cmd.dy = -1;

        if (mon->typ != MON_CAT &&
            mon->typ != MON_RHINO &&
            cmd.dx && cmd.dy) {
            if (rn2(2))
                cmd.dx = 0;
            else
                cmd.dy = 0;
        }
        if (!rn2(20)) {
            cmd.dx = 0;
            cmd.dy = 0;
            if (rn2(2))
                cmd.dx = (rn2(2) ? 1 : -1);
            else
                cmd.dy = (rn2(2) ? 1 : -1);
        }
    }

    switch (cmd.typ) {
    case CMD_HELP:
        pline("Some quick help: h=Left, j=Down, k=Up, l=Right, "
              ",=Pickup, S=quit, .=rest.");
        pline("You can also use the numpad or arrow keys to move "
              "and s to rest.");
        return ACT_FREE;
    case CMD_REST:
        return ACT_DONE;
    case CMD_QUIT:
        return ACT_DIED;
    case CMD_PICKUP:
        obj = obj_at(mon->level, mon->x, mon->y);
        if (!obj) {
            pline("Nothing to pickup!");
            return ACT_FREE; /* nothing here, don't waste a turn */
        }

        pickobj(mon, obj);
        return ACT_DONE;
    case CMD_MOVE:
        return (mon_move(mon, &cmd) ? ACT_DONE : ACT_FREE);
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
mon_at(struct level *level, int x, int y)
{
    struct mon *mon;
    for (mon = level->monlist; mon; mon = mon->nmon)
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

    if (x < 0 || x >= ROOMSIZEX || y < 0 || y >= ROOMSIZEY ||
        has_obstacle(mon->level, x, y)) {
        if (you)
            pline("That's a wall.");
        return false;
    }

    struct mon *mdef = mon_at(mon->level, x, y);
    bool udef = (mdef == &pmon);
    if (mdef) {
        if (--mdef->hp) {
            pline("%s%s hit%s %s%s.",
                  you ? "" : "The ",
                  you ? "You" : mons[mon->typ].name,
                  you ? "" : "s", mon == mdef || udef ? "" : "the ",
                  mon == mdef ? "itself" :
                  udef ? "you" : mons[mdef->typ].name);
            return true;
        }

        mdef->dead = 1;
        mon->kills++;
        pline("%s%s kill%s %s%s who had %d kill%s!",
              you ? "" : "The ",
              you ? "You" : mons[mon->typ].name,
              you ? "" : "s", mon == mdef || udef ? "" : "the ",
              mon == mdef ? "itself" :
              udef ? "you" : mons[mdef->typ].name,
              mdef->kills, mdef->kills == 1 ? "" : "s");
        if (mdef == &pmon) {
            pline("You die...");
            pline("You killed %d monster%s!",
                  mdef->kills, mdef->kills == 1 ? "" : "s");
            pline("Press any key to exit.");
            unsigned dummy;
            timeout_get_wch(-1, &dummy);
        }
        return true;
    }

    mon->x = x;
    mon->y = y;
    return true;
}
