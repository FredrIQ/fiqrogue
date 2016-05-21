/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "obj.h"
#include "rogue.h"
#include "ui.h"

/* Object handling */

/* Returns the object at the given location, if any */
struct obj *
obj_at(int x, int y)
{
    struct obj *obj;
    for (obj = gamestate.objlist; obj; obj = obj->nobj) {
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
    objlist_free(obj);
    free(obj);
}

/* Create an object */
struct obj *
obj_new(enum objtyp typ, int x, int y, bool add_to_objlist)
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

    /* Add to the object list */
    if (add_to_objlist)
        objlist_addlevel(obj);
    return obj;
}

/* Add an object to the main list */
bool
objlist_addlevel(struct obj *obj)
{
    return objlist_add(&gamestate.objlist, obj);
}

/* Add an object to a list. Implies freeing it from elsewhere */
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

    /* Free it from a potential other list */
    objlist_free(obj);

    obj->nobj = *chain;
    *chain = obj;
    return true;
}

/* Frees an object from the chain it is in */
bool
objlist_free(struct obj *obj)
{
    if (!obj)
        return false; /* oops? */

    struct obj **chain = &gamestate.objlist;
    if (obj->carrier)
        chain = &obj->carrier->invent;

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
