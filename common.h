//
// Created by urv on 10/3/20.
//

#ifndef COMMON_H
#define COMMON_H

#define VERSION "0.0.5"

#define SET_OFF   0
#define SET_ON    1
#define NOT_DEF  -1

struct VirtLed {
    int     Led_val;
    char    Led_str[16];

    int     Key_val;
    char    Key_str[16];

    int     newState;

};

int loglevel;

#endif //COMMON_H
