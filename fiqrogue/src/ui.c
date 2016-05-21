/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "mon.h"
#include "obj.h"
#include "rogue.h"
#include "ui.h"

/* UI handling functionality, and the lowerlevel interface to libuncursed */

/* Initialize the interface */
struct WINDOW *
ui_init(void)
{
    uncursed_set_title("FIQRogue");
    struct WINDOW *win = initscr();
    if (!win)
        return NULL; /* something went wrong... */

    set_faketerm_font_file("data/font.png");
    wrefresh(win);
    return win;
}

/* Parses key input and returns in-game commands */
enum cmd
ui_cmd(void)
{
    unsigned key;
    do {
        timeout_get_wch(1000, &key);
    } while (!key);
    switch (key) {
    case 'h': case KEY_LEFT: case KEY_B1:
        return CMD_LEFT;
    case 'j': case KEY_DOWN: case KEY_C2:
        return CMD_DOWN;
    case 'k': case KEY_UP: case KEY_A2:
        return CMD_UP;
    case 'l': case KEY_RIGHT: case KEY_B3:
        return CMD_RIGHT;
    case 'S': /* "save" (or for now, suicide!) */
        return CMD_QUIT;
    }
    return CMD_NONE; /* TODO: write "Unknown command" or similar somewhere */
}

/* Refresh the UI */
void
ui_refresh(void)
{
    /* Display a playfield */
    int x, y;
    for (x = 0; x < ROOMSIZEX; x++)
        for (y = 0; y < ROOMSIZEY; y++)
            mvwaddstr(gamestate.win, x, y, ".");

    /* Place the objects */
    struct obj *obj;
    for (obj = gamestate.objlist; obj; obj = obj->nobj)
        mvwaddstr(gamestate.win, obj->x, obj->y,
                  objcats[objs[obj->typ].cat].letter);

    /* Place the monsters */
    struct mon *mon;
    for (mon = gamestate.monlist; mon; mon = mon->nmon) {
        if (mon_dead(mon))
            continue;

        mvwaddstr(gamestate.win, mon->x, mon->y, mons[mon->typ].letter);
    }
    wmove(gamestate.win, pmon.x, pmon.y);
    wrefresh(gamestate.win);
}
