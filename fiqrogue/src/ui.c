/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "mon.h"
#include "obj.h"
#include "rogue.h"
#include "ui.h"

/* UI handling functionality, and the lowerlevel interface to libuncursed */

struct windows window;

static void strarray_shift(char *[], int);
static void ui_writemessage(const char *, bool);
static void ui_resetmessages(void);

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
    int i;
    for (i = 0; i < NUM_MSGLINES; i++)
        window.msg[i] = NULL;

    ui_reset(false);
}

/* Parses key input and returns in-game commands
   TODO: This could use a second level of abstraction between this and mon_act,
   because we want a way to handle purely interface-based commands, and
   commands that require additional "parameters" (for example: eat item X).
   That way we also avoid returning CMD_NONE to mon_act for resize events,
   something purely user-specific. */
void
ui_cmd(struct command *cmd)
{
    unsigned key = 0;
    do {
        timeout_get_wch(1000, &key);
    } while (!key);
    switch (key) {
    case '?':
        cmd->typ = CMD_HELP;
        break;
    case 'h': case KEY_LEFT: case KEY_B1: /* B1/C2/A2/B3 are numpad 4/2/8/6 */
        cmd->typ = CMD_MOVE;
        cmd->dx = -1;
        break;
    case 'j': case KEY_DOWN: case KEY_C2:
        cmd->typ = CMD_MOVE;
        cmd->dy = 1;
        break;
    case 'k': case KEY_UP: case KEY_A2:
        cmd->typ = CMD_MOVE;
        cmd->dy = -1;
        break;
    case 'l': case KEY_RIGHT: case KEY_B3:
        cmd->typ = CMD_MOVE;
        cmd->dx = 1;
        break;
    case ',':
        cmd->typ = CMD_PICKUP;
        break;
    case 'S': /* "save" (or for now, suicide!) */
        cmd->typ = CMD_QUIT;
        break;
    case KEY_RESIZE:
        ui_reset(true);
        cmd->typ = CMD_NONE;
        break;
    default:
        cmd->typ = CMD_UNKNOWN;
    }
}

/* Print out a message (in printf-format) on the screen */
void
pline(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    strarray_shift(gamestate.msg, NUM_MESSAGES);

    /* We want an appropriate buffer length for the message, so first just
       check the length */
    int len = vsnprintf(NULL, 0, format, args);
    len++; /* vsnprintf return doesn't include the implied null */

    gamestate.msg[NUM_MESSAGES-1] = malloc(len);
    vsnprintf(gamestate.msg[NUM_MESSAGES-1], len, format, args);
    va_end(args);

    ui_writemessage(gamestate.msg[NUM_MESSAGES-1], true);
}

/* Shifts arrays of strings.
   TODO: maybe an util.c for things like this? */
static void
strarray_shift(char *array[], int length)
{
    free(array[0]);

    int i;
    for (i = 1; i < length; i++)
        array[i-1] = array[i];

    /* Free the last one */
    array[length-1] = NULL;
}

/* Called on resizes (and redraws), processes the message log for outputting
   from scratch. */
static void
ui_resetmessages(void)
{
    /* Kill existing message buffers */
    int i;
    for (i = 0; i < NUM_MSGLINES; i++) {
        free(window.msg[i]);
        window.msg[i] = NULL;
    }

    for (i = 0; i < NUM_MESSAGES; i++)
        ui_writemessage(gamestate.msg[i], false);
}

/* Add a message to the screen */
static void
ui_writemessage(const char *msg, bool update_window)
{
    if (!msg)
        return;

    int msglen = strlen(msg);
    const char *winmsg = window.msg[NUM_MSGLINES-1];
    int winlen = winmsg ? strlen(winmsg) : 0;
    char buffer[msglen + 1];
    strcpy(buffer, msg);
    int offset = 0;

    /* Check if we need to scroll, possibly several times */
    while (winlen + msglen + 2 >= COLS) {
        strarray_shift(window.msg, NUM_MSGLINES);
        if (winlen) {
            winlen = 0;
            if (msglen < COLS)
                break; /* Having its own line was enough to fit */
        }

        unsigned breakpoint = COLS;
        bool breakword = true; /* In case we can't wrap on space */
        int i;
        for (i = 0; i < COLS; i++) {
            if (buffer[i + offset] == ' ') {
                breakpoint = i + 1;
                breakword = false;
            }
        }

        /* Add part of the string to the message window and free up the now
           redundant part of msg */
        char *new = malloc(breakpoint);
        strncpy(new, buffer + offset, breakpoint-1);
        new[breakpoint-1] = '\0'; /* add null terminator */
        window.msg[NUM_MSGLINES-1] = new;

        offset += breakpoint;
        if (breakword)
            offset--; /* Don't skip over the breakpoint for word-break */
        msglen -= offset;
    }

    /* If winlen is still set here, that means that no scroll was needed.
       Add 2 spaces inbetween the messages */
    if (winlen)
        winlen += 2;

    window.msg[NUM_MSGLINES-1] = realloc(window.msg[NUM_MSGLINES-1],
                                         winlen + msglen + 1);
    if (winlen) {
        strcat(window.msg[NUM_MSGLINES-1], "  ");
        strcat(window.msg[NUM_MSGLINES-1], buffer + offset);
    } else
        strcpy(window.msg[NUM_MSGLINES-1], buffer + offset);

    if (update_window)
        ui_refresh();
    return;
}

/* Resets the interface, redoing (or possibly doing, this is also called on
   first run) all the windows from scratch. Called on window resizing.
   Setting output_screen to false allows one to avoid an implied ui_refresh,
   which can be useful if the level content isn't ready. */
void
ui_reset(bool output_screen)
{
    /* Kill existing windows */
    if (window.msg)
        delwin(window.msgarea);
    if (window.level)
        delwin(window.level);
    /* TODO
    if (window.menu)
    ui_killmenus(window.menu); */

    /* Ensure that we have at least an 80x24 area */
    while (LINES < 24 || COLS < 80) {
        const char *toosmall = "fiqrogue needs at least 80x24 to function.";
        if (LINES >= 1 && COLS >= strlen(toosmall))
            mvwaddstr(window.root, 0, 0, toosmall);
        unsigned key = 0;
        do {
            timeout_get_wch(-1, &key);
        } while (key != KEY_RESIZE);
    }
    wclear(window.root); /* Blanks the window */
    wrefresh(window.root);

    /* For now, the only thing that actually changes on resize is height of
       different areas, the width is always "the entire screen". */
    int top = 0, height = 0;

    /* Calculate message area height */
    height = LINES - ROOMSIZEY;
    if (height > NUM_MSGLINES)
        height = NUM_MSGLINES;
    window.msgarea = newwin(height, COLS, top, 0);

    /* The game area, which is below the message area */
    top += height;
    window.level = newwin(ROOMSIZEY, ROOMSIZEX, top, 0);

    if (output_screen) {
        ui_resetmessages();
        ui_refresh();
    }
}

/* Refresh the UI */
void
ui_refresh(void)
{
    /* Display a playfield */
    int x, y;
    for (x = 0; x < ROOMSIZEX; x++)
        for (y = 0; y < ROOMSIZEY; y++)
            mvwaddstr(window.level, y, x, ".");

    /* Place the objects */
    struct obj *obj;
    for (obj = gamestate.objlist; obj; obj = obj->nobj)
        mvwaddstr(window.level, obj->y, obj->x,
                  objcats[objs[obj->typ].cat].letter);

    /* Place the monsters */
    struct mon *mon;
    for (mon = gamestate.monlist; mon; mon = mon->nmon) {
        if (mon_dead(mon))
            continue;

        mvwaddstr(window.level, mon->y, mon->x, mons[mon->typ].letter);
    }

    /* Display messages */
    werase(window.msgarea);
    int i;
    for (i = 0; i < NUM_MSGLINES; i++) {
        if (NUM_MSGLINES - CUR_MSGLINES > i)
            continue;

        if (window.msg[i])
            mvwaddstr(window.msgarea, i - (NUM_MSGLINES - CUR_MSGLINES),
                      0, window.msg[i]);
    }

    wmove(window.level, pmon.y, pmon.x);
    wrefresh(window.msgarea);
    wrefresh(window.level);
}
