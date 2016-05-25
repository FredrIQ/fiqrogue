/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef RANDOM_H
# define RANDOM_H

# include <stdlib.h>
# include <time.h>

/* Randomness functions, header */

extern void rnd_init(void);
extern int rn2(int);
extern int rnd(int);
extern int d(int, int);

#endif /* RANDOM_H */
