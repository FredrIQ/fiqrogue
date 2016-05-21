/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef MON_H
# define MON_H

# include "mondata.h"

/* Actions */
enum act {
    ACT_FREE,
    ACT_DONE,
    ACT_DIED,
};

struct mon {
    int x;
    int y;
    bool dead;
    enum montyp typ;
    struct object *invent; /* inventory */
    struct mon *nmon;
};

extern enum act mon_act(struct mon *);
#define mon_dead(mon) ((mon)->dead)
extern void mon_free(struct mon *);
extern struct mon *mon_new(enum montyp, int, int, bool);
extern bool monlist_add(struct mon *);
extern bool monlist_free(struct mon *);

#endif /* MON_H */
