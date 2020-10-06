#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>

#include "log.h"
#include "args.h"

#define VERSION "0.0.1a"

const char short_options[] = "hic:n:s:D:";

const struct option
        long_options[] = {
        { "help",        no_argument,       NULL, 'h' },
        { "info",        no_argument,       NULL, 'i' },
        { "capslock",    required_argument, NULL, 'c' },
        { "numlock",     required_argument, NULL, 'n' },
        { "scrolllock",  required_argument, NULL, 's' },
        { "debug",       required_argument, NULL, 'D' },
        { 0, 0, 0, 0 }
};


void usage(char* file_name) {
    fprintf(stderr, "Version %s \n", VERSION);
    fprintf(stderr, "Usage: %s --capslock [on/off] --numlock [on/off] --scrolllock[on/off]"
                    " [--info] [--debug] \n\n",  file_name);

    fprintf(stderr,"Options: \n");
    fprintf(stderr, "\t --info          Info about all LEDs on all keyboards \n");
    fprintf(stderr, "\t --capslock      Set CapsLock LED  \n");
    fprintf(stderr, "\t --numlock       Set NumLock LED \n");
    fprintf(stderr, "\t --scrolllock    Set ScrollLock LED \n");
    fprintf(stderr, "\t --help          Print this message \n");
    fprintf(stderr, "\t --debug         Debug level [0..6] \n");
}


void pars_args(int argc, char **argv, int* show_info,
               struct VirtLed *Caps_inst, struct VirtLed *Num_inst, struct VirtLed *Scroll_inst)
{
    int loglevel;

    if( argc == 1 ) {
        usage(argv[0]);
        exit(0);
    }

    for (;;) {
        int idx;
        int c;
        c = getopt_long(argc, argv, short_options, long_options, &idx);

        if( c == -1 )
            break;

        switch (c) {
            case 0: // getopt_long() flag
                break;

            case 'i':
                *show_info = 1;
                log_trace("Show LEDs info on all keyboards");
                break;

            case 'c':
                Caps_inst->inWork = 1;
                if (strcmp(optarg, "on") == 0 )
                    Caps_inst->newState = SET_ON;
                else
                    Caps_inst->newState = SET_OFF;
                break;

            case 'n':
                Num_inst->inWork = 1;
                if (strcmp(optarg, "on") == 0 )
                    Num_inst->newState = SET_ON;
                else
                    Num_inst->newState = SET_OFF;
                break;

            case 's':
                Scroll_inst->inWork = 1;
                if (strcmp(optarg, "on") == 0 )
                    Scroll_inst->newState = SET_ON;
                else
                    Scroll_inst->newState = SET_OFF;
                break;

            case 'D':
                loglevel = (int) strtol(optarg, NULL, 10);
                if( loglevel < LOG_TRACE || loglevel > LOG_FATAL ) {
                    log_fatal("A problem with parameter '--debug'");
                    exit(-1);
                }
                log_set_level(loglevel);
                break;

            case 'h':
            default:
                usage(argv[0]);
                exit(0);
        }
    }
}
