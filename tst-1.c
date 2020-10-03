//
// Created by urv on 9/26/20.
//
// https://askubuntu.com/questions/321512/how-to-set-or-clear-usb-keyboard-leds
//

#define _GNU_SOURCE     // for 'versionsort' support
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/input.h>

#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
    int fd;
    int retval;

    //fd = open("//dev/input/event8", O_RDWR );
    fd = open("/dev/input/event21", O_RDWR );
    //char kbname [256] = "key-x123";
    //char LedStatus = 0;
    /*
    ioctl (fd, EVIOCGNAME (sizeof (kbname)), kbname);
    printf("kbname: %s \n", kbname);

    ioctl (fd, EVIOCGBIT (EV_LED,sizeof (LedStatus)), &LedStatus);
    printf("LedStatus: %d \n", LedStatus);

    ioctl (fd, EVIOCGLED (sizeof (LedStatus)), &LedStatus);
    printf("LedStatus: %d \n", LedStatus);
    */
    struct input_event ev; /* the event */

/* we turn off all the LEDs to start */

    //ev.type = EV_LED;
    //ev.code = LED_CAPSL;
    //ev.value = 0;
    //write(fd, &ev, sizeof(struct input_event));

    ev.type = EV_KEY;
    ev.code = KEY_CAPSLOCK;
    ev.value = 1;
    write(fd, &ev, sizeof(struct input_event));

    ev.type = EV_KEY;
    ev.code = KEY_CAPSLOCK;
    ev.value = 0;
    write(fd, &ev, sizeof(struct input_event));

    ev.type = EV_LED;
    ev.code = LED_CAPSL;
    ev.value = 1;
    write(fd, &ev, sizeof(struct input_event));


    /*
    ev.code = LED_NUML;
    retval = write(fd, &ev, sizeof(struct input_event));
    ev.code = LED_SCROLLL;
    retval = write(fd, &ev, sizeof(struct input_event));
    */

    close(fd);
}

