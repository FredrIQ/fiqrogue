/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "mon.h"
#include "obj.h"
#include "rogue.h"
#include "ui.h"

/* Object handling */

void
pickobj(struct mon *mon, struct obj *obj)
{
    objlist_add(&mon->invent, obj);
    obj->carrier = mon;
}

/* Returns the object at the given location, if any */
struct obj *
obj_at(struct level *level, int x, int y)
{
    struct obj *obj;
    for (obj = level->objlist; obj; obj = obj->nobj) {
        if (obj->x == x && obj->y == y)
            return obj;
    }
    return NULL;
}

/* Deallocate an object */
void
obj_free(struct obj *obj)
{
    if (!obj)
        return;

    free(obj);
}

/* Create an object */
struct obj *
obj_new(enum objtyp typ, int x, int y)
{
    struct obj *obj = malloc(sizeof (struct obj));
    memset(obj, 0, sizeof (struct obj));
    if (x >= ROOMSIZEX)
        x = ROOMSIZEX - 1;
    if (y >= ROOMSIZEY)
        y = ROOMSIZEY - 1;

    obj->x = x;
    obj->y = y;
    if (typ > LAST_OBJ)
        typ = LAST_OBJ;
    obj->typ = typ;

    return obj;
}

/* Add an object to a list. */
bool
objlist_add(struct obj **chain, struct obj *obj)
{
    if (!obj)
        return false; /* shouldn't happen */

    struct obj *otmp;
    for (otmp = *chain; otmp; otmp = otmp->nobj) {
        if (obj == otmp)
            return false; /* already in the list */
    }

    obj->nobj = *chain;
    *chain = obj;
    return true;
}

/* Frees an object from the chain it is in */
bool
objlist_free(struct obj **chain, struct obj *obj)
{
    if (!obj)
        return false; /* oops? */

    struct obj *prevobj = NULL;
    struct obj *listobj = NULL;
    for (listobj = *chain; listobj; listobj = listobj->nobj) {
        if (obj != listobj) {
            prevobj = listobj;
            continue;
        }

        if (prevobj)
            prevobj->nobj = listobj->nobj;
        else
            *chain = listobj->nobj;
        return true; /* we're done here */
    }
    return false; /* not in the list */
}
