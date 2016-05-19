/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/

#include "game.h"
#include "rogue.h"

/* Main program initialization */

int
main(int argc, char *argv[])
{
    /* This needs to be done before messing with command arguments, because
       it can possibly screw with them due to uncursed-specific arguments */
    initialize_uncursed(&argc, argv);

    /* Parse commandline flags before we initialize uncursed proper, because
       some of them will ultimately just print to stdout and nothing else. */
    while (argc && argv[0][1] == '-') {
        argc--;
        argv++;
        /* "--" means "ignore further flags" in a lot of contexts, retain this
           assumption here in case we later want to, for example, allow one
           to specify a file name (say, for a save/config) and not confuse
           people used to calling conventions */
        if (!strcmp(argv[0], "--"))
            break;

        switch (argv[0][1]) {
        case 'h': /* help */
            puts("Usage: fiqrogue [-h]");
            break;
        }
    }

    struct WINDOW *win = ui_init();
    if (!win) {
        puts("Unable to initialize libuncursed.");
        exit(EXIT_FAILURE);
    }

    game_init(win);
    game_loop();
    endwin();
}
