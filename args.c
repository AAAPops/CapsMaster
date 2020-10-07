#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>

#include "args.h"
#include "log.h"
#include "common.h"

//const char short_options[] = "hic:n:s:d:";
const char short_options[] = "d:";

const struct option
        long_options[] = {
        { "help",        no_argument,       NULL, 'h' },
        { "info",        no_argument,       NULL, 'i' },
        { "capslock",    required_argument, NULL, 'c' },
        { "numlock",     required_argument, NULL, 'n' },
        { "scrolllock",  required_argument, NULL, 's' },
        { "debug",       required_argument, NULL, 'd' },
        { 0, 0, 0, 0 }
};


void usage(char* file_name) {
    fprintf(stderr, "Version %s \n", VERSION);
    fprintf(stderr, "Usage: %s --capslock [on/off] --numlock [on/off] --scrolllock[on/off]"
                    " [--info] [--debug] \n\n",  file_name);

    fprintf(stderr,"Options: \n");
    fprintf(stderr, "    --info          Info about all LEDs on all keyboards \n");
    fprintf(stderr, "    --capslock      Set CapsLock LED  \n");
    fprintf(stderr, "    --numlock       Set NumLock LED \n");
    fprintf(stderr, "    --scrolllock    Set ScrollLock LED \n");
    fprintf(stderr, "    --help          Print this message \n");
    fprintf(stderr, " -d|--debug         Debug level [0, 2, 5] \n");
}


void pars_args(int argc, char **argv, int* show_info,
               struct VirtLed *CapsInst, struct VirtLed *NumInst, struct VirtLed *ScrollInst)
{
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
                break;

            case 'c':
                if (strcmp(optarg, "on") == 0 )
                    CapsInst->newState = SET_ON;
                else
                    CapsInst->newState = SET_OFF;
                break;

            case 'n':
                if (strcmp(optarg, "on") == 0 )
                    NumInst->newState = SET_ON;
                else
                    NumInst->newState = SET_OFF;
                break;

            case 's':
                if (strcmp(optarg, "on") == 0 )
                    ScrollInst->newState = SET_ON;
                else
                    ScrollInst->newState = SET_OFF;
                break;

            case 'd':
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
