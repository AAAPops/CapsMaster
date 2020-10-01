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

#include "utils.h"



#define DEV_INPUT_EVENT "/dev/input"
#define EVENT_DEV_NAME  "event"

#define MAX_MOUSE_DEVS 10

enum {
    MAX_WIDTH  = 1024,
    MAX_HEIGHT = 768,
    VALUE_PRESSED  = 1,
    VALUE_RELEASED = 0,
};



/**
 * Filter for the AutoDevProbe scandir on /dev/input.
 *
 * @param dir The current directory entry provided by scandir.
 *
 * @return Non-zero if the given directory entry starts with "event", or zero
 * otherwise.
 */
static int is_event_device(const struct dirent *dir) {
    return strncmp(EVENT_DEV_NAME, dir->d_name, 5) == 0;
}



/*
 * Search 'event*' files in '/dev/input/' directory
 * and test witch one is Mouse devices
 *
 * Output: array of strings with records:
 * '/dev/input/event5'
 * '/dev/input/event22'
 */
int find_mouse_dev(char *ev_name[], int ev_name_max)
{
    struct dirent **namelist;
    int iter;
    int ndev;
    int ev_name_iter = 0;
    int retval = -1;

    uint32_t ev_type;
    uint8_t ev_code[KEY_MAX/8 + 1];

    ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, versionsort);
    if (ndev <= 0)
        return -1;

    for (iter = 0; iter < ndev; iter++) {
        char fname[64];
        int fd = -1;

        snprintf(fname, sizeof(fname), "%s/%s", DEV_INPUT_EVENT, namelist[iter]->d_name);
        printf("Check device: %s \n", fname);
        fd = open(fname, O_RDONLY);
        if (fd < 0) {
            printf("--- 0 --- \n");
            continue;
        }


        printf("--- 1 --- \n");
        if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_type)), &ev_type) < 0) {
            perror("evdev ioctl(EVIOCGBIT)");
            retval = -1;
            goto out;
        }

        printf("--- 2 --- \n");
        if( BIT_CHECK(ev_type, EV_KEY) ) {
            memset(ev_code, 0, sizeof(ev_code));

            if( ioctl(fd, EVIOCGBIT(EV_KEY, (KEY_MAX/8+1)), ev_code) < 0 ) {
                perror("evdev ioctl(EVIOCGBIT) ");
                retval = -1;
                goto out;
            }

            // BTN_MOUSE = 0x110 == 272, so we have to check bit #272 (34 * 8 = 272)
            if( BIT_CHECK( *(ev_code + 34), 0) )
            {
                sprintf(ev_name[ev_name_iter], fname);
                retval = ++ev_name_iter;

                if( ev_name_iter == ev_name_max ) {
                    close(fd);
                    goto out;
                }
            }
        }

        close(fd);
    }


    out:
    for (iter = 0; iter < ndev; iter++)
        free(namelist[iter]);
    free(namelist);

    return retval;
}



/*
 * Set file descriptors for mouse devices
 * '/dev/input/event5'  ==> 4
 * '/dev/input/event22' ==> 5
 *
 * Output: Number of mouse file descriptors
 *
 */
int mouse_open_dev(int *fd_arr) {
    int iter;
    int opened_n_mouse = 0;

    // allocate space for N pointers to strings
    char **strings = (char**)malloc(MAX_MOUSE_DEVS * sizeof(char*));
    //char *strings[MAX_MOUSE_EVS];

    //allocate space for each string
    for(iter = 0; iter < MAX_MOUSE_DEVS; iter++)
        strings[iter] = (char*)malloc(256 * sizeof(char));

    int ret = find_mouse_dev(strings, MAX_MOUSE_DEVS);
    if( ret < 1 ) {
        printf("Mouse device not found! \n");
        exit(-1);
    }

    printf("Find mouse events: \n");
    for( iter = 0; iter < ret; iter++ ) {
        fd_arr[iter] = open(strings[iter], O_RDONLY);
        opened_n_mouse++;

        printf("   %s [fd: %d] \n", strings[iter], fd_arr[iter] );

        free(strings[iter]);
    }
    free(strings);


    return opened_n_mouse;
}

int main() {
    int mouse_fd;
    int mouse_fd_n;

    mouse_fd_n = mouse_open_dev(&mouse_fd);


}




