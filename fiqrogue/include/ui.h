/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef UI_H
# define UI_H

# include "rogue.h"
# include "uncursed.h"

enum cmd {
    CMD_HELP,
    CMD_MOVE,
    CMD_PICKUP,
    CMD_QUIT,
    CMD_UNKNOWN,
    CMD_NONE, /* Used for things like resizing */
};

struct command {
    enum cmd typ;
    unsigned x; /* Specific x/y */
    unsigned y;
    int dx; /* Relative x/y (do X upwards/etc) */
    int dy;
    char letter; /* Inventory prompts, etc */
};

/* NUM_MSGLINES is the upper limit. In practice the limit can be smaller as a
   result of a smaller game area. */
# define CUR_MSGLINES getmaxy(window.msgarea)
# define NUM_MSGLINES 10
# define NUM_MESSAGES 50

/* 52 covers A-Z, a-z. More than that shouldn't really be needed for now. */
# define NUM_MENULINES 52

enum menutyp {
    MEN_YESNO, /* y/n prompt */
    MEN_TEXT, /* text box */
    MEN_LIST, /* simple list */
    MEN_ABC, /* letter-choice list */
    MEN_ABCMANY, /* multi-choice letter list */
    MEN_ABCMANYCOUNT, /* multi-choice letter list allowing a limited selection */
};

enum menualign {
    MAL_LEFT,
    MAL_RIGHT,
    MAL_TOP,
    MAL_BOTTOM,
    MAL_CENTER,
};

# define MENUCHOICE_MARKED 0x80000000u

/* Menu metadata. Submenus are built with a linked list. */
struct winmenu {
    WINDOW *win;
    enum menutyp typ;
    enum menualign alignx;
    enum menualign aligny;
    unsigned top;
    unsigned left;
    bool scrollable;
    unsigned scrollpos; /* Scroll offset of the menu */
    unsigned cursorpos; /* Position of the cursor, if the user prefers specifying things
                           that way. */
    char lettermap[NUM_MENULINES];
    /* choicetable is an array with selected choices in an ABCMANY list.
       The most significant bit determines the actual choice, while
       lower bits determine how many the user picked of that item, in case
       of a multichoice list where count is allowed.
       The applicable choicetable will also be marked in a simple abc menu,
       if acquiring the choice that way is preferred for some reason, for
       consistency. */
    unsigned choicetable[NUM_MENULINES];
    char *header; /* Also the only line used in yesno/text menu types */
    char *input; /* for MEN_TEXT */
    char *line[NUM_MENULINES];
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

extern void ui_init(void);
extern void ui_cmd(struct command *);
extern bool confirm(const char *);
extern struct winmenu *uimenu_init(enum menutyp, enum menualign, enum menualign,
                                   int, int, const char *);
extern bool uimenu_input(struct winmenu *, char *);
extern void uimenu_delete(struct winmenu *, bool);
extern void pline(const char *, ...);
extern void ui_reset(bool);
extern void ui_refresh(void);

#endif /* UI_H */
