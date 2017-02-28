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
    int kills;
    int hp;
    bool dead;
    enum montyp typ;
    struct obj *invent; /* inventory */
    struct level *level;
    struct mon *nmon;
};

#define mon_dead(mon) ((mon)->dead)
extern void mon_free(struct mon *);
extern struct mon *mon_new(struct level *, enum montyp, int, int);
extern bool monlist_add(struct level *, struct mon *);
extern bool monlist_free(struct mon *);
extern enum act mon_act(struct mon *);
extern struct mon *mon_at(struct level *, int, int);

#endif /* MON_H */
