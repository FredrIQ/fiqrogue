/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef GAME_H
# define GAME_H

# include "mon.h"
# include "ui.h"

struct gamestate {
    struct mon *monlist; /* monster list */
    struct obj *objlist; /* object list */
    unsigned vismap[ROOMSIZEX][ROOMSIZEY];
    char *msg[NUM_MESSAGES]; /* message log */
};

extern struct gamestate gamestate;
extern struct mon pmon;

extern void game_init(void);
extern void game_loop(void);

#endif /* GAME_H */
