/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef VISION_H
# define VISION_H

# include "rogue.h"

# define MAP_LIT 1
# define MAP_VISIBLE 2

extern void fov_recalc(void);
extern bool has_obstacle(int, int);

#endif
