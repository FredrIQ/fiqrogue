/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "level.h"
#include "mon.h"
#include "obj.h"
#include "rogue.h"
#include "ui.h"
#include "vision.h"

/* UI handling functionality, and the lowerlevel interface to libuncursed */

struct windows window;

static void strarray_shift(char *[], int);
static void ui_writemessage(const char *, bool);
static void ui_resetmessages(void);
static bool uimenu_input_inner(struct winmenu *, char *);
static void uimenu_populate(struct winmenu *, bool);

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

    /* Kill the game on hangup unconditionally for now */
    if (key == KEY_HANGUP)
        exit(EXIT_SUCCESS);

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
        cmd->typ = confirm("Are you sure?") ? CMD_QUIT : CMD_NONE;
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

    /* Check if we need to scroll, possibly several times. +2 is to include
       double-space inbetween messages */
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

bool
confirm(const char *text)
{
    struct winmenu *menu;
    menu = uimenu_init(MEN_YESNO, MAL_CENTER, MAL_CENTER, 3, 0, text);
    wrefresh(menu->win);

    char confirm;
    if (uimenu_input(menu, &confirm, true))
        return (confirm == 'y' ? true : false);

    return false; /* user pressed escape */
}

/* Initializes a new menu. alignx/aligny are suggested alignment,
   if top/left specifics are requested, use those in place of the aligns.
   Returns the resulting winmenu unless an error occured in which NULL
   is returned (Most likely top/left being out of bounds).
   The defined top/left includes borders */
struct winmenu *
uimenu_init(enum menutyp typ, enum menualign alignx, enum menualign aligny,
            int top, int left, const char *header)
{
    /* Technically top==0 and/or left==0 is valid, but so are ALIGN_LEFT, ALIGN_TOP which
       does the same thing in a more straightforward way. */
    if (top < 0 || top >= LINES || left < 0 || left >= COLS ||
        (alignx != MAL_LEFT && alignx != MAL_RIGHT && alignx != MAL_CENTER &&
         aligny != MAL_TOP && aligny != MAL_BOTTOM && aligny != MAL_CENTER))
        return NULL; /* improper parameters */

    struct winmenu *menu;
    if (!window.menu) {
        window.menu = malloc(sizeof (struct winmenu));
        menu = window.menu;
    } else {
        /* Get the last entry in the list */
        struct winmenu *next;
        for (menu = window.menu; next; menu = next)
            next = menu->next;

        menu->next = malloc(sizeof (struct winmenu));
        menu = menu->next;
    }

    memset(menu, 0, sizeof (struct winmenu));
    menu->typ = typ;
    menu->top = top;
    menu->left = left;
    menu->alignx = alignx;
    menu->aligny = aligny;

    int len = strlen(header);
    len++;
    menu->header = malloc(len);
    strcpy(menu->header, header);
    menu->header[len-1] = '\0';

    uimenu_populate(menu, false);
    return menu;
}

/* Menu input.
   Returns true on completion or false if the user escaped the menu.
   letter is a pointer to a character (not a string) and will hold the
   user's response if it was a character based one ('y'/'n' for yes/no prompts,
   the character choice for simple abc menus). For complex abc menus and for text
   input, the choice will be part of the menu struct. */
bool
uimenu_input(struct winmenu *menu, char *letter, bool delete_on_completion)
{
    bool ret = uimenu_input_inner(menu, letter);
    if (delete_on_completion)
        uimenu_delete(menu, true);

    return ret;
}
static bool
uimenu_input_inner(struct winmenu *menu, char *letter)
{
    unsigned key = 0;
    while (true) {
        timeout_get_wch(-1, &key);
        if (key == KEY_HANGUP)
            exit(EXIT_SUCCESS);

        if (key == KEY_ESCAPE)
            return false;

        switch (menu->typ) {
        case MEN_YESNO:
            if (key == 'y' || key == 'n') {
                *letter = key;
                return true;
            }
            break;
        default:
            break;
        }
    }
}

/* Populates a menu.
   Note that if the needed height/width *decreased*, the UI should be reset to avoid
   graphical glitches. */
static void
uimenu_populate(struct winmenu *menu, bool output_screen)
{
    if (!menu)
        return;

    /* Destroy existing window if applicable */
    if (menu->win) {
        werase(menu->win);
        delwin(menu->win);
        menu->win = NULL;
    }

    /* Figure out how large and wide the menu must be.
       height is allowed to overflow (in which a scrollable area is used),
       if width overflows, text will be cut off. */
    int height = 0; /* Borders are added last */
    int width = 0;
    int len = 0;

    if (menu->typ == MEN_TEXT) {
        height++; /* Prompt line */
        width = MAXSTRING; /* Input string length */
    }

    if (menu->header) {
        height++; /* Header line */
        len = strlen(menu->header);
        if (menu->typ == MEN_YESNO)
            len += 8; /* Adds room for " (y/n)  " */
        if (width < len)
            width = len;
    }

    int i;
    for (i = 0; i < NUM_MENULINES; i++) {
        if (!menu->line[i])
            break;
        height++;

        len = strlen(menu->line[i]);
        if (menu->typ == MEN_ABC ||
            menu->typ == MEN_ABCMANY ||
            menu->typ == MEN_ABCMANYCOUNT)
            len += 4; /* Adds room for "a - " */
        if (len > width)
            width = len;
    }

    /* Add borders */
    height += 2;
    width += 2;

    int top = menu->top;
    int left = menu->left;

    /* Figure out if corrections need to be made */
    menu->scrollable = false;
    if (!top) {
        if (menu->aligny == MAL_TOP)
            top = 0; /* essentially no-op, but is here to make it clear */
        else if (menu->aligny == MAL_BOTTOM)
            top = LINES - height;
        else {
            int centery = LINES / 2;
            top = centery - (height / 2);
        }

        if (top < 0) { /* the menu ended up too large, use scroll */
            top = 0;
            menu->scrollable = true;
        }
    }

    if (!left) {
        if (menu->alignx == MAL_LEFT)
            left = 0;
        else if (menu->alignx == MAL_RIGHT)
            left = COLS - width;
        else {
            int centerx = COLS / 2;
            left = centerx - (width / 2);
        }

        if (left < 0)
            left = 0;
    }

    if (top + height > LINES) {
        if (height > LINES) {
            height = LINES;
            menu->scrollable = true;
            top = 0;
        } else
            top = LINES - height;
    }
    if (left + width > COLS) {
        if (width > LINES) {
            width = LINES;
            left = 0;
        } else
            left = COLS - width;
    }

    /* Now we have the proper dimensions. Create the window and populate it. */
    menu->win = newwin(height, width, top, left);

    int line = 1; /* which effective line to print out on */

    int offset = 0;
    switch (menu->typ) {
    case MEN_YESNO:
        if (menu->header) {
            mvwaddstr(menu->win, line, 1, menu->header);
            offset += strlen(menu->header) + 1;
        }

        mvwaddstr(menu->win, line, offset + 1, "(y/n) ");
        break;
    case MEN_TEXT:
        if (menu->header) {
            mvwaddstr(menu->win, line, 1, menu->header);
            line++;
        }
        mvwaddstr(menu->win, line, 1, "[TODO: Implement input area]");
        break;
    case MEN_ABC:
    case MEN_ABCMANY:
    case MEN_ABCMANYCOUNT:
        offset += 4; /* Needs room for "a - " */
    case MEN_LIST:
        if (menu->header) {
            mvwaddstr(menu->win, line, 1, menu->header);
            line++;
        }

        for (i = (menu->scrollable ? menu->scrollpos : 0);
                  i < NUM_MENULINES; i++) {
            /* Clear the line in case a message overflow, which can happen if the window
               isn't wide enough. Since borders are drawn last, the potential of a text
               overflowing to them isn't a problem, it'll be overwritten anyway. */
            wmove(menu->win, line, 0);
            wclrtobot(menu->win);

            if (!menu->line[i] || line == height)
                break; /* Out of lines or area to print */

            mvwaddstr(menu->win, line, offset + 1, menu->line[i]);
            if (menu->typ != MEN_LIST) { /* It's a choice table, so add special stuff */
                char letter[2];
                letter[0] = menu->lettermap[i];
                letter[1] = '\0';
                mvwaddstr(menu->win, line, 1, letter);

                const char *choicesym = "-";
                if ((menu->choicetable[i] & MENUCHOICE_MARKED)) {
                    choicesym = "+";
                    /* Use # to mark numbered choices */
                    if ((menu->choicetable[i] & ~MENUCHOICE_MARKED))
                        choicesym = "#";
                }
                mvwaddstr(menu->win, line, 3, choicesym);
                if (menu->cursorpos == i)
                    mvwaddstr(menu->win, line, 4, ">");
            }
            line++;
        }
        break;
    }

    wborder_set(menu->win, WACS_VLINE, WACS_VLINE,
                WACS_HLINE, WACS_HLINE,
                WACS_ULCORNER, WACS_URCORNER,
                WACS_LLCORNER, WACS_LRCORNER);

    if (output_screen) {
        wmove(menu->win, height, width);
        wrefresh(menu->win);
    }
}

/* Deletes a menu. Implies deleting children. */
void
uimenu_delete(struct winmenu *menu, bool output_screen)
{
    /* Iterate menus, but we want to set the previous menu pointer addr to NULL too which
       slightly complicates the loop */
    bool foundmenu = false;
    struct winmenu *menutmp;
    struct winmenu *menunext;
    for (menutmp = window.menu; menutmp; menutmp = menunext) {
        menunext = menutmp->next;
        if (!foundmenu) {
            if (menutmp == menu) { /* menu is window.menu */
                window.menu = NULL;
                foundmenu = true; /* no continue, since this is what we want to free */
            } else if (menunext == menu) {
                menutmp->next = NULL;
                foundmenu = true;
                continue;
            } else
                continue;
        }

        free(menu->header);
        free(menu->input);
        int i;
        for (i = 0; i < NUM_MENULINES; i++)
            free(menu->line[i]);

        if (menu->win)
            delwin(menu->win);

        free(menu);
    }

    if (output_screen)
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
        //ui_resetmenus();
        ui_resetmessages();
        ui_refresh();
    }
}

/* Refresh the UI */
void
ui_refresh(void)
{
    fov_recalc();
    struct level *level = pmon.level;

    /* Display a playfield */
    werase(window.level);

    int x, y;
    for (x = 0; x < ROOMSIZEX; x++) {
        for (y = 0; y < ROOMSIZEY; y++) {
            if ((level->prop[x][y] & MAP_VISIBLE))
                mvwaddstr(window.level, y, x,
                          has_obstacle(x, y) ? "#" : ".");
        }
    }

    /* Place the objects */
    struct obj *obj;
    for (obj = level->objlist; obj; obj = obj->nobj)
        mvwaddstr(window.level, obj->y, obj->x,
                  objcats[objs[obj->typ].cat].letter);

    /* Place the monsters */
    struct mon *mon;
    for (mon = level->monlist; mon; mon = mon->nmon) {
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
