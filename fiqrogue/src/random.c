/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "random.h"

/* Randomness functions. */

/* Seeds the random number generator. (Use this instead of srand() directly
   to allow for more straightforward tweaks later) */

void
rnd_init(void)
{
    srand(time(NULL));
}

/* Returns a random number in the range [0, max-1] */
int
rn2(int max) {
    if (max > RAND_MAX)
        return 0; /* Too high input */

    int ret;
    do {
        ret = rand();
    } while (ret >= RAND_MAX - (RAND_MAX % max));

    return ret % max;
}

/* Returns a random number in the range [1, max] */
int
rnd(int max) {
    return rn2(max) + 1;
}

/* Returns a random value simulating rolling numdice dices of size dicesize */
int
d(int numdice, int dicesize)
{
    int ret = 0;
    int i;
    for (i = 0; i < numdice; i++)
        ret += rnd(dicesize);

    return ret;
}
