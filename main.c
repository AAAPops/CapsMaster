// https://www.linuxjournal.com/article/6429

#define _GNU_SOURCE     // for 'versionsort' support
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/input.h>

#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

#include "common.h"
#include "utils.h"
#include "log.h"
#include "args.h"


#define DEV_EVENT_DIR   "/dev/input"
#define NAME_FILTER     "event"


#define EV_TYPE_BYTE_MAX    EV_CNT/8   //  0x1f+1 = 32;    32/8 = 4 byte;
#define EV_KEY__BYTE_MAX    KEY_CNT/8   // 0x2ff+1 = 768;  768/8 = 96 byte;
#define EV_LED__BYTE_MAX    LED_CNT/8   // 0x0f+1 = 16;    16/8 = 2 byte;


char* LED_state[] = {"OFF", "ON"};


/**
 * Filter for the AutoDevProbe scandir on /dev/input.
 *
 * @param dir The current directory entry provided by scandir.
 *
 * @return Non-zero if the given directory entry starts with "event", or zero
 * otherwise.
 */
static int name_filter(const struct dirent *dir) {
    return strncmp(NAME_FILTER, dir->d_name, 5) == 0;
}


static int bit_check_in_bitarr(uint8_t* byte_arr, uint32_t bit_num) {

    return BIT_CHECK(byte_arr[bit_num/8], bit_num%8);
}

//--------------//--------------//--------------//--------------//
int has_device_LED(int fd, int led_name)
{
    uint8_t ev_type_bitarr[EV_TYPE_BYTE_MAX];
    uint8_t ev_led__bitarr[EV_LED__BYTE_MAX];

    int retval = 0;


    if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_type_bitarr)), ev_type_bitarr) < 0) {
        perror("evdev ioctl(EVIOCGBIT:0)");
        return -1;
    }


    if( bit_check_in_bitarr(ev_type_bitarr, EV_LED) ) {
        printf("\t...EV_LED device found!!! \n");
        memset(ev_led__bitarr, 0, sizeof(ev_led__bitarr));


        if( ioctl(fd, EVIOCGBIT(EV_LED, sizeof(ev_led__bitarr)), ev_led__bitarr) < 0 ) {
            perror("evdev ioctl(EVIOCGBIT:EV_KEY) ");
            retval = -1;
        }

        if( bit_check_in_bitarr(ev_led__bitarr, led_name) ) {
            printf("\t...'%d' device found!!! \n", led_name);
            retval = 1;
        }
    }

    return retval;
}

int set_LED(int fd, int led_name, int set_to_state) {

    uint8_t ev_led__bitarr[EV_LED__BYTE_MAX];
    int curr_state;
    struct input_event ev; /* the event */

    // get current LED state
    {
        ioctl(fd, EVIOCGLED(sizeof(ev_led__bitarr)), ev_led__bitarr);

        curr_state = bit_check_in_bitarr(ev_led__bitarr, led_name);
        printf("\t...current state '%d' is '%s' \n", led_name, LED_state[curr_state]);
    }


    if (curr_state != set_to_state) {
        ev.type = EV_KEY;
        ev.code = KEY_CAPSLOCK;
        ev.value = 1;
        write(fd, &ev, sizeof(struct input_event));

        ev.type = EV_KEY;
        ev.code = KEY_CAPSLOCK;
        ev.value = 0;
        write(fd, &ev, sizeof(struct input_event));

        if (set_to_state == SET_ON) {
            ev.type = EV_LED;
            ev.code = led_name;
            ev.value = 1;
            write(fd, &ev, sizeof(struct input_event));
        } else {
            ev.type = EV_LED;
            ev.code = led_name;
            ev.value = 0;
            write(fd, &ev, sizeof(struct input_event));
        }
    }

    // get current LED state again
    {
        ioctl(fd, EVIOCGLED(sizeof(ev_led__bitarr)), ev_led__bitarr);

        curr_state = bit_check_in_bitarr(ev_led__bitarr, led_name);
        printf("\t.....new state '%d' is '%s' \n", led_name, LED_state[curr_state]);
    }

    return 0;
}



int main(int argc, char **argv) {
    // Массив, содержащий список файлов в директории
    struct dirent **namelist;
    struct Args_inst args_I ;
    MEMZERO(args_I);

    int retcode = 0;
    int dev_n;
    int idx;
    char file_long_name[1024];
    int kbd_fd;

    int led_caps_present;
    int key_caps_present;

    pars_args(argc, argv, &args_I);


    dev_n = scandir(DEV_EVENT_DIR, &namelist, name_filter, versionsort);
    if (dev_n <= 0) {
        retcode = -1;
        goto out;
    }

    for (idx = 0; idx < dev_n; idx++) {
        snprintf(file_long_name, sizeof(file_long_name), "%s/%s", DEV_EVENT_DIR, namelist[idx]->d_name);
        printf("Check device: %s \n", file_long_name);

        kbd_fd = open(file_long_name, O_RDWR);
        if (kbd_fd < 0) {
            retcode = -1;
            goto out;
        }

        led_caps_present = has_device_LED(kbd_fd, LED_CAPSL);

        if (led_caps_present == 1) {
            set_LED(kbd_fd, LED_CAPSL, SET_ON);
        }
    }



    out:
    for (idx = 0; idx < dev_n; idx++)
        free(namelist[idx]);
    free(namelist);

    return retcode;
}




