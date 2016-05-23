/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef UI_H
# define UI_H

# include "rogue.h"
# include "uncursed.h"

enum cmd {
    CMD_HELP,
    CMD_LEFT,
    CMD_DOWN,
    CMD_UP,
    CMD_RIGHT,
    CMD_PICKUP,
    CMD_QUIT,
    CMD_UNKNOWN,
    CMD_NONE, /* Used for things like resizing */
};

/* NUM_MSGLINES is the upper limit. In practice the limit can be smaller as a result of a
   smaller game area. */
# define NUM_MSGLINES 10
# define NUM_MESSAGES 50

/* winmenu is a simple linked list containing (sub)menus */
struct winmenu {
    WINDOW *win;
    struct winmenu *next;
};

/* All game windows are stored in a struct windows named just "windows" */
struct windows {
    WINDOW *root; /* stdscr */
    WINDOW *msgarea; /* message area */
    WINDOW *level; /* level area */
    char *msg[NUM_MSGLINES]; /* message lines as they appear on the screen */
    struct winmenu *menu;
};

extern struct windows window;

# define CUR_MSGLINES getmaxy(window.msgarea)

extern void ui_init(void);
enum cmd ui_cmd(void);
extern void pline(const char *, ...);
extern void ui_reset(bool);
extern void ui_refresh(void);

#endif /* UI_H */
