/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef MONDATA_H
# define MONDATA_H

# include "rogue.h"

/* TODO: Find a better way to store monster metadata */

/* montyp and actual data list must be in the same order... */
enum montyp {
    MON_NONE,
    MON_PLAYER,
    MON_JACKAL,
    LAST_MON = MON_JACKAL,
};

struct mondata {
    char name[MAXSTRING];
    char letter[2];
};

static const struct mondata mons[] = {
    {"dummy", " "},
    {"player", "@"},
    {"jackal", "j"},
};

#endif /* MONDATA_H */
