/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#ifndef UI_H
# define UI_H

# include "uncursed.h"

enum cmd {
    CMD_LEFT,
    CMD_DOWN,
    CMD_UP,
    CMD_RIGHT,
    CMD_PICKUP,
    CMD_QUIT,
    CMD_NONE,
};

extern struct WINDOW *ui_init(void);
enum cmd ui_cmd(void);
extern void ui_refresh(void);

#endif /* UI_H */
