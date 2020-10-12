// https://www.linuxjournal.com/article/6429

#define _GNU_SOURCE     // for 'versionsort' support
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/input.h>

#include <string.h>
#include <fcntl.h>
#include <dirent.h>

#include "common.h"
#include "utils.h"
#include "log.h"
#include "args.h"


#define DEV_EVENT_DIR   "/dev/input"
#define NAME_FILTER     "event"

#define KBD_FD_MAX 10

#define EV_TYPE_BYTE_MAX    EV_CNT/8   //  0x1f+1 = 32;    32/8 = 4 byte;
#define EV_KEY__BYTE_MAX    KEY_CNT/8   // 0x2ff+1 = 768;  768/8 = 96 byte;
#define EV_LED__BYTE_MAX    LED_CNT/8   // 0x0f+1 = 16;    16/8 = 2 byte;


char* LED_state[] = {"OFF", "ON"};


static int name_filter(const struct dirent *dir) {
    return strncmp(NAME_FILTER, dir->d_name, 5) == 0;
}


/*!
 * @param byte_arr bytes in memory
 * @param bit_num bit number in byte_arr
 *
 * @return  0 Bit set to 0
 * @return  1 Bit set to 1
 */
static int bit_check_in_bitarr(uint8_t* byte_arr, uint32_t bit_num) {

    return BIT_CHECK(byte_arr[bit_num/8], bit_num%8);
}


/*!
 * @return  0 LED is OFF
 * @return  1 LED is ON
 * @return -1 system call Error
 */
int get_LED_state (int fd, int led_val, char* led_name) {

    log_trace("%s()", __FUNCTION__ );

    uint8_t ev_led__bitarr[EV_LED__BYTE_MAX];
    int curr_state;

    // get current LED state
    if (ioctl(fd, EVIOCGLED(sizeof(ev_led__bitarr)), ev_led__bitarr) < 0) {
        log_warn("ioctl(EVIOCGLED)");
        return -1;
    }

    curr_state = bit_check_in_bitarr(ev_led__bitarr, led_val);
    log_trace("\t'%s' is '%s'", led_name, LED_state[curr_state]);
    if (curr_state == 1)
        return 1;
    else
        return 0;
}


/*!
 * @return  0 if LED present on keyboard
 * @return  1 if NOT present on keyboard
 * @return -1 system call Error
 */
int is_LED_on_kbd(int fd, int led_val, char* led_name)
{
    log_trace("%s()", __FUNCTION__ );

    uint8_t ev_type_bitarr[EV_TYPE_BYTE_MAX];
    uint8_t ev_led__bitarr[EV_LED__BYTE_MAX];

    if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_type_bitarr)), ev_type_bitarr) < 0) {
        log_warn("ioctl(EVIOCGBIT:0)");
        return -1;
    }

    if( ! bit_check_in_bitarr(ev_type_bitarr, EV_LED) ) {
        log_trace("\tEV_LED device Not found!");
        return 1;
    }


    memset(ev_led__bitarr, 0, sizeof(ev_led__bitarr));

    if( ioctl(fd, EVIOCGBIT(EV_LED, sizeof(ev_led__bitarr)), ev_led__bitarr) < 0 ) {
        log_warn("ioctl(EVIOCGBIT:EV_LED)");
        return -1;
    }

    if( bit_check_in_bitarr(ev_led__bitarr, led_val) ) {
        log_trace("\t'%s' found!", led_name);
        return 0;
    } else {
        log_trace("\t'%s' not presen!", led_name);
        return 1;
    }
}


int press_Key(int fd, int key_val, char* key_name) {

    struct input_event ev;

    ev.type = EV_KEY;
    ev.code = key_val;
    ev.value = 1;
    write(fd, &ev, sizeof(struct input_event));

    ev.type = EV_KEY;
    ev.code = key_val;
    ev.value = 0;
    write(fd, &ev, sizeof(struct input_event));

    log_info("Key %s pressed", key_name);

    return 0;
}


//--------------//--------------//--------------//--------------//
int set_LED(int fd, int led_val, char* led_name, int led_state) {

    int curr_state;
    struct input_event ev;

    ev.type = EV_LED;
    ev.code = led_val;
    ev.value = led_state;
    write(fd, &ev, sizeof(struct input_event));

    log_info("Led %s set to '%s'", led_name, LED_state[led_state]);

    return 0;
}


/*!
 * @return >= 0 Number of keyboard present in system
 * @return -1 System call error
 */
int find_all_keyboards(int *kbd_fd_arr, struct VirtLed *CapsInst,
                       struct VirtLed *NumInst, struct VirtLed *ScrollInst){

    log_trace("%s()", __FUNCTION__ );

    // Array that has all file's name in dir
    struct dirent **namelist;
    char file_long_name[1024];

    int idx;
    int dev_n;
    int tmp_fd;
    int kbd_fd_count = 0;

    /*
     * Will think that device is a keyboard if 'CapsLock' LED are present!!!
     */
    dev_n = scandir(DEV_EVENT_DIR, &namelist, name_filter, versionsort);
    if (dev_n <= 0) {
        log_fatal("Can not open devices in '%s' dir", DEV_EVENT_DIR);
        return -1;
    }

    for (idx = 0; idx < dev_n; idx++) {
        snprintf(file_long_name, sizeof(file_long_name), "%s/%s", DEV_EVENT_DIR, namelist[idx]->d_name);
        log_trace("Check device: %s", file_long_name);

        tmp_fd = open(file_long_name, O_RDWR);
        if (tmp_fd < 0) {
            log_warn("Can not open device: %s", file_long_name);
            close(tmp_fd);
            continue;
        }


        if ( is_LED_on_kbd(tmp_fd, CapsInst->Led_val, CapsInst->Led_str) ) {
            close(tmp_fd);
            continue;
        }

        if ( is_LED_on_kbd(tmp_fd, NumInst->Led_val, NumInst->Led_str) ) {
            close(tmp_fd);
            continue;
        }

        if ( is_LED_on_kbd(tmp_fd, ScrollInst->Led_val, ScrollInst->Led_str) ) {
            close(tmp_fd);
            continue;
        }

        kbd_fd_arr[kbd_fd_count] = tmp_fd;
        kbd_fd_count++;
        log_trace("\tDevice '%s' is a keyboard [fd: %d]\n", file_long_name, tmp_fd);
    }

    for (idx = 0; idx < dev_n; idx++)
        free(namelist[idx]);
    free(namelist);

    return kbd_fd_count;
}


/*!
 * @return 0 OK
 * @return -1 System call Error
 */
int show_kbd_curr_state(int fd, struct VirtLed *Caps_inst,
        struct VirtLed *Num_inst, struct VirtLed *Scroll_inst) {

    char kbd_name[256]= "Unknown type";
    int tmp_state;

    //static char LED_absent[] = "---";
    char *curr_caps_state, *curr_num_state, *curr_scroll_state;

    if(ioctl(fd, EVIOCGNAME(sizeof(kbd_name)), kbd_name) < 0) {
        log_fatal("ioctl(EVIOCGNAME)");
        return -1;
    }


    tmp_state = get_LED_state(fd, Caps_inst->Led_val, Caps_inst->Led_str);
    curr_caps_state = LED_state[tmp_state];


    tmp_state = get_LED_state(fd, Num_inst->Led_val, Num_inst->Led_str);
    curr_num_state = LED_state[tmp_state];


    tmp_state = get_LED_state(fd, Scroll_inst->Led_val, Scroll_inst->Led_str);
    curr_scroll_state = LED_state[tmp_state];

    log_info("Keyboard: '%s' ===> CapsLock(%s) --- NumLock(%s) --- ScrollLock(%s)", kbd_name,
             curr_caps_state, curr_num_state, curr_scroll_state);

    return 0;
}


//===============//===============//===============//===============//
int main(int argc, char **argv) {

    log_set_level(LOG_FATAL);
    loglevel = NOT_DEF;
    int show_kbd_info = -1;

    struct VirtLed Caps = {.Led_val = LED_CAPSL, .Led_str = "CapsLock",
            .Key_val = KEY_CAPSLOCK, .Key_str = "CapsLock", .newState = NOT_DEF};

    struct VirtLed Num = {.Led_val = LED_NUML, .Led_str = "NumLock",
            .Key_val = KEY_NUMLOCK, .Key_str = "NumLock", .newState = NOT_DEF};

    struct VirtLed Scroll = {.Led_val = LED_SCROLLL, .Led_str = "ScrollLock",
            .Key_val = KEY_SCROLLLOCK, .Key_str = "ScrollLock", .newState = NOT_DEF};

    int kbd_fd_arr[KBD_FD_MAX] = {0};
    int kbd_in_system = 0;


    // Parsing input arguments
    pars_args(argc, argv, &show_kbd_info, &Caps, &Num, &Scroll);

    if (Caps.newState != NOT_DEF)
        log_info("app option: Set %s LED to '%s'", Caps.Led_str, LED_state[Caps.newState]);
    if (Num.newState != NOT_DEF)
        log_info("app option: Set %s LED to '%s'", Num.Led_str, LED_state[Num.newState]);
    if (Scroll.newState != NOT_DEF)
        log_info("app option: Set %s LED to '%s'", Scroll.Led_str, LED_state[Scroll.newState]);
    if (show_kbd_info == 1) {
        if (loglevel == NOT_DEF)
            log_set_level(LOG_INFO);
        log_info("app option: '--info' route", show_kbd_info);
    }
    //------------------------------------------


    kbd_in_system = find_all_keyboards(kbd_fd_arr, &Caps, &Num, &Scroll);
    if (kbd_in_system == 0 ) {
        log_fatal("There is no keyboard attached to the computer!!!");
        return 0;
    }
    log_info("Found '%d' keyboard(s) in the system", kbd_in_system);
    for (int idx=0; idx < kbd_in_system; idx++)
        log_trace("kbd fd: %d", kbd_fd_arr[idx]);



    /* Show all info about keyboard */
    if (show_kbd_info == 1) {
        for (int idx=0; idx < kbd_in_system; idx++) {
            show_kbd_curr_state(kbd_fd_arr[idx], &Caps, &Num, &Scroll);

            close (kbd_fd_arr[idx]);
        }

        return 0;
    }


    /* Set LED as user wish */
    log_info("---");
    for (int idx=0; idx < 1; idx++)
    {
        int curr_state;

        curr_state = get_LED_state(kbd_fd_arr[idx], Caps.Led_val, Caps.Led_str);
        if (Caps.newState == NOT_DEF)
            Caps.newState = curr_state;

        if (curr_state != Caps.newState) {
            press_Key(kbd_fd_arr[idx], Caps.Key_val, Caps.Key_str);
        } else {
            press_Key(kbd_fd_arr[idx], Caps.Key_val, Caps.Key_str);
            press_Key(kbd_fd_arr[idx], Caps.Key_val, Caps.Key_str);
        }
        set_LED(kbd_fd_arr[idx], Caps.Led_val, Caps.Led_str, Caps.newState);


        curr_state = get_LED_state(kbd_fd_arr[idx], Num.Led_val, Num.Led_str);
        if (Num.newState == NOT_DEF)
            Num.newState = curr_state;

        if (curr_state != Num.newState) {
            press_Key(kbd_fd_arr[idx], Num.Key_val, Num.Key_str);
        } else {
            press_Key(kbd_fd_arr[idx], Num.Key_val, Num.Key_str);
            press_Key(kbd_fd_arr[idx], Num.Key_val, Num.Key_str);
        }
        set_LED(kbd_fd_arr[idx], Num.Led_val, Num.Led_str, Num.newState);


        curr_state = get_LED_state(kbd_fd_arr[idx], Scroll.Led_val, Scroll.Led_str);
        if (Scroll.newState == NOT_DEF)
            Scroll.newState = curr_state;

        if (curr_state != Scroll.newState) {
            press_Key(kbd_fd_arr[idx], Scroll.Key_val, Scroll.Key_str);
        } else {
            press_Key(kbd_fd_arr[idx], Scroll.Key_val, Scroll.Key_str);
            press_Key(kbd_fd_arr[idx], Scroll.Key_val, Scroll.Key_str);
        }
        set_LED(kbd_fd_arr[idx], Scroll.Led_val, Scroll.Led_str, Scroll.newState);


        //press_Key(kbd_fd_arr[idx], KEY_ENTER, "KEY_ENTER");
    }

    if (loglevel >= LOG_TRACE && loglevel <= LOG_INFO) {
        log_info("After set LED...");
        for (int idx=0; idx < kbd_in_system; idx++) {
            usleep(20000);
            show_kbd_curr_state(kbd_fd_arr[idx], &Caps, &Num, &Scroll);
        }
    }

    for (int idx=0; idx < kbd_in_system; idx++)
        close (kbd_fd_arr[idx]);

    return 0;
}

