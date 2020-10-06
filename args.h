#ifndef INCLUDE_ARGS_H
#define INCLUDE_args_H

#include <stdio.h>
#include <stdint.h>
#include <getopt.h>

#include "common.h"


void pars_args(int argc, char **argv, int* show_info,
               struct VirtLed *Caps_inst, struct VirtLed *Num_inst, struct VirtLed *Scroll_inst);


#endif