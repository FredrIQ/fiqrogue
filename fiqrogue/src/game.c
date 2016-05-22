/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "mon.h"
#include "obj.h"
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

    /* Resets the user interface, which creates all windows from scratch */
    ui_reset(false);

    /* We want to assign a proper monster struct to the player, but mon_new()
       will malloc additional memory which we don't care about in this case.
       Store this result in mon_tmp so we can free it properly */
    struct mon *mon_tmp = mon_new(MON_PLAYER, 10, 10, false);
    pmon = *mon_tmp;
    monlist_add(&pmon);
    free(mon_tmp); /* now get rid of it */

    /* Create another monster */
    mon_new(MON_JACKAL, 10, 10, true);

    /* Create a weapon */
    obj_new(OBJ_SWORD, 20, 20, true);

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
        for (mon = gamestate.monlist; mon; mon = nmon) {
            nmon = mon->nmon;
            if (mon_dead(mon)) {
                mon_free(mon);
                continue;
            }

            /* ACT_FREE is a free action, so continue until
               an action that takes time is performed */
            act = ACT_FREE;
            while (act == ACT_FREE)
                act = mon_act(mon);

            switch (act) {
            case ACT_DIED: /* died */
                if (mon == &pmon) /* player died */
                    return; /* this will abort the game */

                mon_free(mon);
            case ACT_DONE:
                break;
            default:
                break; /* TODO: error handling */
            }
        }

        /* Reset the interface */
        ui_refresh();
    }
}
