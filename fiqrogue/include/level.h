/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef LEVEL_H
# define LEVEL_H

# include "mon.h"
# include "rogue.h"

/* Level management */

# define NUM_LEVELS 127

/* An important thing to consider: the T-shaped wall borders are inconsistently named in
   uncursed (probably resulting from ncurses) -- T and _|_ are top/bottom, but 
   |-/-| is right/left. This inconsistency is kept here to avoid even more confusion.
   XWALL is tiles supposed to be walls but that hasn't gone through wallification.
   (XTWALL is just the + linedrawing character) */
# define MAP_XWALL       0x00000001
# define MAP_HWALL       0x00000002
# define MAP_VWALL       0x00000003
# define MAP_ULWALL      0x00000004
# define MAP_URWALL      0x00000005
# define MAP_LLWALL      0x00000006
# define MAP_LRWALL      0x00000007
# define MAP_TTWALL      0x00000008
# define MAP_BTWALL      0x00000009
# define MAP_RTWALL      0x0000000a
# define MAP_LTWALL      0x0000000b
# define MAP_XTWALL      0x0000000c
# define MAP_LAST_WALL   0x0000000c
# define MAP_ROCK        0x0000000d
# define MAP_LAST_SOLID  0x0000000f
# define MAP_FLOOR       0x00000010
# define MAP_CORRIDOR    0x00000011
# define MAP_TYPEMASK    0x000000ff
# define MAP_LIT         0x00000100
# define MAP_VISIBLE     0x00000200
# define MAP_EXPLORED    0x00000400

struct level {
    unsigned prop[ROOMSIZEX][ROOMSIZEY]; /* Map properties */
    int z; /* Floor level, starts from 1 and goes up as you go deeper */
    struct mon *monlist;
    struct obj *objlist;
};

extern struct level *level_init(int);
#endif /* LEVEL_H */
