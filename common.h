//
// Created by urv on 10/3/20.
//

#ifndef COMMON_H
#define COMMON_H

#define SET_OFF 0
#define SET_ON  1

struct Args_inst {
    int     capsLed;
    int     capsKey;
    int     capsState;

    int     numLed;
    int     numKey;
    int     numState;

    int     scrollLed;
    int     scrollKey;
    int     scrollState;

    int     info;
};

#endif //COMMON_H
