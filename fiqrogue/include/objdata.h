/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef OBJDATA_H
# define OBJDATA_H

# include "rogue.h"

/* enums and data must be in the same order... */
enum objcat {
    OBC_NONE,
    OBC_WEAPON,
    LAST_OBC = OBC_WEAPON,
};

enum objtyp {
    OBJ_NONE,
    OBJ_SWORD,
    OBJ_DAGGER,
    LAST_OBJ = OBJ_DAGGER,
};

struct objcategory {
    char name[MAXSTRING];
    char letter[2];
};

static const struct objcategory objcats[] = {
    {"dummy", " "},
    {"Weapons", ")"},
};

struct objdata {
    char name[MAXSTRING];
    enum objcat cat;
};

static const struct objdata objs[] = {
    {"dummy", OBC_NONE},
    {"sword", OBC_WEAPON},
    {"dagger", OBC_WEAPON},
};

#endif /* OBJDATA_H */
