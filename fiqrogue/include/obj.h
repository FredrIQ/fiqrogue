/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef OBJ_H
# define OBJ_H

# include "mon.h"
# include "objdata.h"

struct obj {
    int x; /* x/y is not accurate if the object isn't on the floor */
    int y;
    enum objtyp typ;
    char invlet; /* inventory letter */
    struct mon *carrier;
    struct obj *nobj;
};

extern void pickobj(struct mon *, struct obj *);
extern struct obj *obj_at(struct level *, int, int);
extern void obj_free(struct obj *);
extern struct obj *obj_new(enum objtyp, int, int);
extern bool objlist_add(struct obj **, struct obj *);
extern bool objlist_free(struct obj **, struct obj *);

#endif /* OBJ_H */
