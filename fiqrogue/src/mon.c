/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "mon.h"
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
    enum cmd cmd;
    if (mon == &pmon)
        cmd = ui_cmd();
    else
        cmd = (mon->y == 30 ? CMD_LEFT : CMD_RIGHT); /* dummy AI */

    if (cmd == CMD_NONE)
        return ACT_FREE; /* TODO: "unknown command" */

    if (cmd == CMD_QUIT)
        return ACT_DIED; /* we "died" */
    if (cmd == CMD_LEFT ||
        cmd == CMD_DOWN ||
        cmd == CMD_UP ||
        cmd == CMD_RIGHT) {
        mvwaddstr(gamestate.win, mon->x, mon->y, ".");
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
        mvwaddstr(gamestate.win, mon->x, mon->y, "@");
        wrefresh(gamestate.win);
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
mon_new(int x, int y, bool add_to_monlist)
{
    struct mon *mon = malloc(sizeof (struct mon));
    memset(mon, 0, sizeof (struct mon));
    if (x >= ROOMSIZEX)
        x = ROOMSIZEX - 1;
    if (y >= ROOMSIZEY)
        y = ROOMSIZEY - 1;

    mon->x = x;
    mon->y = y;

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
