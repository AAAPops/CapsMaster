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


void pars_args(int argc, char **argv, struct Args_inst *Args_i)
{
    char* LED_state[] = {"OFF", "ON"};

    int loglevel = LOG_FATAL;
    log_set_level(loglevel);

    if( argc == 1 ) {
        usage(argv[0]);
        exit(0);
    }

    //----->> Set defaults  <<-----//
    Args_i->capsLed = -1;
    Args_i->capsKey = -1;
    Args_i->capsState = SET_OFF;

    Args_i->numLed = -1;
    Args_i->numKey = -1;
    Args_i->numState = SET_OFF;

    Args_i->scrollLed = -1;
    Args_i->scrollKey = -1;
    Args_i->scrollState = SET_OFF;

    Args_i->info = -1;
    //----->> Set defaults  <<-----//

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
                log_trace("Show LEDs info on all keyboards");
                break;

            case 'c':
                Args_i->capsLed = LED_CAPSL;
                Args_i->capsKey = KEY_CAPSLOCK;
                if (strcmp(optarg, "on") == 0 )
                    Args_i->capsState = SET_ON ;
                break;

            case 'n':
                Args_i->numLed = LED_NUML;
                Args_i->numKey = KEY_NUMLOCK;
                if (strcmp(optarg, "on") == 0 )
                    Args_i->numState = SET_ON ;
                break;

            case 's':
                Args_i->scrollLed = LED_SCROLLL;
                Args_i->scrollKey = KEY_SCROLLLOCK;
                if (strcmp(optarg, "on") == 0 )
                    Args_i->scrollState = SET_ON ;
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

    if (Args_i->capsLed >= 0)
        log_trace("Set CapsLock LED: %s", LED_state[Args_i->capsState]);

    if (Args_i->numLed >= 0)
        log_trace("Set NumLock LED: %s", LED_state[Args_i->numState]);

    if (Args_i->scrollLed >= 0)
        log_trace("Set ScrollLock LED: %s", LED_state[Args_i->scrollState]);
}
