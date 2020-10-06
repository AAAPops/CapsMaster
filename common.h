//
// Created by urv on 10/3/20.
//

#ifndef COMMON_H
#define COMMON_H

#define SET_OFF 0
#define SET_ON  1

struct VirtLed {
    int     inWork;
    int     presentOnKbd;

    int     Led_val;
    char    Led_str[16];

    int     Key_val;
    char    Key_str[16];

    int     currState;
    int     newState;

};

#endif //COMMON_H
