/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "mon.h"
#include "obj.h"
#include "rogue.h"
#include "ui.h"

/* UI handling functionality, and the lowerlevel interface to libuncursed */

struct windows window;

/* Initialize the interface */
void
ui_init(void)
{
    uncursed_set_title("FIQRogue");
    struct WINDOW *win = initscr();
    if (!win) {
        puts("Unable to initialize libuncursed.");
        exit(EXIT_FAILURE);
    }

    set_faketerm_font_file("data/font.png");
    wrefresh(win);
    window.root = win;
}

/* Parses key input and returns in-game commands */
enum cmd
ui_cmd(void)
{
    unsigned key = 0;
    do {
        timeout_get_wch(1000, &key);
    } while (!key);
    switch (key) {
    case 'h': case KEY_LEFT: case KEY_B1: /* B1/C2/A2/B3 are numpad 4/2/8/6 */
        return CMD_LEFT;
    case 'j': case KEY_DOWN: case KEY_C2:
        return CMD_DOWN;
    case 'k': case KEY_UP: case KEY_A2:
        return CMD_UP;
    case 'l': case KEY_RIGHT: case KEY_B3:
        return CMD_RIGHT;
    case ',':
        return CMD_PICKUP;
    case 'S': /* "save" (or for now, suicide!) */
        return CMD_QUIT;
    case KEY_RESIZE:
        ui_reset(true);
        return CMD_NONE;
    }
    return CMD_NONE; /* TODO: write "Unknown command" or similar somewhere */
}

/* Resets the interface, redoing (or possibly doing, this is also called on first run) all
   the windows from scratch. Called on window resizing.
   Setting output_screen to false allows one to avoid an implied ui_refresh, which
   can be useful if the level content isn't ready. */
void
ui_reset(bool output_screen)
{
    /* Kill existing windows */
    if (window.msg)
        delwin(window.msg);
    if (window.level)
        werase(window.level);
    /* TODO
    if (window.menu)
    ui_killmenus(window.menu); */

    /* Ensure that we have at least an 80x24 area */
    while (LINES < 24 || COLS < 80) {
        /* Ensure that we at least have enough screen area to draw out an error
           message... If not, just patiently wait until the user sets a sane size.
           The string was counted to the same length as COLS minimum. If it's changed,
           change the minimum COLS. */
        if (LINES >= 1 && COLS >= 42)
            mvwaddstr(window.root, 0, 0, "fiqrogue needs at least 80x24 to function.");
        unsigned key = 0;
        do {
            timeout_get_wch(-1, &key);
        } while (key != KEY_RESIZE);
    }
    wclear(window.root); /* Blanks the window */
    wrefresh(window.root);

    /* For now, the only thing that actually changes is height. */
    int x = 0, y = 0, h = 0, w = 0;

    /* The message area uses up whatever space isn't taken by the level, capped at 10. */
    h = LINES - ROOMSIZEX;
    if (h > 10)
        h = 10;
    window.msg = subwin(window.root, h, w, x, y);
    x += h;
    window.level = subwin(window.root, ROOMSIZEX, ROOMSIZEY, x, y);

    if (output_screen)
        ui_refresh();
}

/* Refresh the UI */
void
ui_refresh(void)
{
    /* Display a playfield */
    int x, y;
    for (x = 0; x < ROOMSIZEX; x++)
        for (y = 0; y < ROOMSIZEY; y++)
            mvwaddstr(window.level, x, y, ".");

    /* Place the objects */
    struct obj *obj;
    for (obj = gamestate.objlist; obj; obj = obj->nobj)
        mvwaddstr(window.level, obj->x, obj->y,
                  objcats[objs[obj->typ].cat].letter);

    /* Place the monsters */
    struct mon *mon;
    for (mon = gamestate.monlist; mon; mon = mon->nmon) {
        if (mon_dead(mon))
            continue;

        mvwaddstr(window.level, mon->x, mon->y, mons[mon->typ].letter);
    }
    wmove(window.level, pmon.x, pmon.y);
    wrefresh(window.level);
}
