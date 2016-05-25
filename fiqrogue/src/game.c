/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "mon.h"
#include "obj.h"
#include "random.h"
#include "rogue.h"
#include "ui.h"

/* "Lowlevel" handling of the game such as globals, game loop and init */

struct mon pmon; /* the player */
struct gamestate gamestate; /* holds misc things */

/* Initialize the game state */
void
game_init(void)
{
    ui_init();
    rnd_init();
    struct level *level = level_init(1);

    /* We want to assign a proper monster struct to the player, but mon_new()
       will malloc additional memory which we don't care about in this case.
       Store this result in mon_tmp so we can free it properly */
    struct mon *mon_tmp = mon_new(level, MON_PLAYER, ROOMSIZEX, ROOMSIZEY);
    pmon = *mon_tmp;
    mon_free(mon_tmp); /* now get rid of it */

    pmon.level = level;
    monlist_add(pmon.level, &pmon);

    /* Create another monster */
    mon_new(pmon.level, MON_JACKAL, ROOMSIZEX, ROOMSIZEY);

    /* Create a weapon */
    struct obj *obj = obj_new(OBJ_SWORD, 20, 20);
    objlist_add(&pmon.level->objlist, obj);

    pline("Welcome to fiqrogue! Use hjkl, arrow keys or the numpad to move.");
    pline("Press ? for help.");
    ui_refresh();
}

/* Main game loop */
void
game_loop(void)
{
    struct mon *mon;
    struct mon *nmon; /* We need to do this in case mon is freed below */
    enum act act;

    while (!mon_dead(&pmon)) {
        /* Create a new monster sometimes */
        if (!rn2(10)) {
            enum montyp mtyp = MON_JACKAL;
            if (!rn2(2)) {
                mtyp = MON_CAT;
                if (!rn2(2))
                    mtyp = MON_RHINO;
            }

            mon_new(pmon.level, mtyp, ROOMSIZEX, ROOMSIZEY);
        }

        /* Currently, a monster can't cause another one to switch monlist
           chain (change dungeon level), so this approach works. If this
           is changed later, we need a new method of iterating the
           monlist. */
        for (mon = pmon.level->monlist; mon; mon = nmon) {
            nmon = mon->nmon;
            if (mon_dead(mon)) {
                mon_free(mon);
                continue;
            }

            /* ACT_FREE is a free action, so continue until
               an action that takes time is performed */
            act = ACT_FREE;
            while (act == ACT_FREE ||
                   (mon->typ == MON_RHINO && rn2(2)))
                act = mon_act(mon);

            switch (act) {
            case ACT_DIED: /* died */
                if (mon == &pmon) /* player died */
                    return; /* this will abort the game */

                mon_free(mon);
            case ACT_DONE:
                break;
            default:
                pline("Unknown monster action result: %d",
                      act);
                break;
            }
        }

        /* Reset the interface */
        ui_refresh();
    }
}
