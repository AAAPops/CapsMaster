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

#define KBD_FD_MAX 10

#define EV_TYPE_BYTE_MAX    EV_CNT/8   //  0x1f+1 = 32;    32/8 = 4 byte;
#define EV_KEY__BYTE_MAX    KEY_CNT/8   // 0x2ff+1 = 768;  768/8 = 96 byte;
#define EV_LED__BYTE_MAX    LED_CNT/8   // 0x0f+1 = 16;    16/8 = 2 byte;


char* LED_state[] = {"OFF", "ON"};


static int name_filter(const struct dirent *dir) {
    return strncmp(NAME_FILTER, dir->d_name, 5) == 0;
}


static int bit_check_in_bitarr(uint8_t* byte_arr, uint32_t bit_num) {

    return BIT_CHECK(byte_arr[bit_num/8], bit_num%8);
}

int get_current_LED_state (int fd, int led_val, char* led_name) {

    uint8_t ev_led__bitarr[EV_LED__BYTE_MAX];
    int curr_state;

    // get current LED state
    if (ioctl(fd, EVIOCGLED(sizeof(ev_led__bitarr)), ev_led__bitarr) < 0) {
        log_warn("ioctl(EVIOCGLED)");
        return -1;
    }

    curr_state = bit_check_in_bitarr(ev_led__bitarr, led_val);
    log_trace("\t '%s' is '%s' \n", led_name, LED_state[curr_state]);
    if (curr_state == 1)
        return 1;
    else
        return 0;
}


//--------------//--------------//--------------//--------------//
int has_device_LED(int fd, int led_val, char* led_name)
{
    uint8_t ev_type_bitarr[EV_TYPE_BYTE_MAX];
    uint8_t ev_led__bitarr[EV_LED__BYTE_MAX];


    if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_type_bitarr)), ev_type_bitarr) < 0) {
        log_warn("ioctl(EVIOCGBIT:0)");
        return -1;
    }

    if( bit_check_in_bitarr(ev_type_bitarr, EV_LED) ) {
        log_trace("\t EV_LED device found!!!");
        memset(ev_led__bitarr, 0, sizeof(ev_led__bitarr));

        if( ioctl(fd, EVIOCGBIT(EV_LED, sizeof(ev_led__bitarr)), ev_led__bitarr) < 0 ) {
            log_warn("ioctl(EVIOCGBIT:EV_LED)");
            return -1;
        }

        if( bit_check_in_bitarr(ev_led__bitarr, led_val) ) {
            log_trace("\t '%s' device found!!!", led_name);
            return 1;
        } else {
            log_trace("\t '%s' device not found!!!", led_name);
            return -1;
        }
    }
}

//--------------//--------------//--------------//--------------//
int set_LED(int fd, struct VirtLed *VirtLedInst) {

    int curr_state;
    struct input_event ev;

    // get current LED state
    VirtLedInst->currState = get_current_LED_state(fd, VirtLedInst->Led_val, VirtLedInst->Led_str);


    if (VirtLedInst->currState != VirtLedInst->newState) {
        ev.type = EV_KEY;
        ev.code = VirtLedInst->Key_val;
        ev.value = 1;
        write(fd, &ev, sizeof(struct input_event));

        ev.type = EV_KEY;
        ev.code = VirtLedInst->Key_val;
        ev.value = 0;
        write(fd, &ev, sizeof(struct input_event));

        if (VirtLedInst->newState == SET_ON) {
            ev.type = EV_LED;
            ev.code = VirtLedInst->Led_val;
            ev.value = 1;
            write(fd, &ev, sizeof(struct input_event));
        } else {
            ev.type = EV_LED;
            ev.code = VirtLedInst->Led_val;
            ev.value = 0;
            write(fd, &ev, sizeof(struct input_event));
        }
    }

    return 0;
}


//--------------//--------------//--------------//--------------//
int find_all_keyboards(int *kbd_fd_arr, struct VirtLed *Caps_inst){

    // Массив, содержащий список файлов в директории
    struct dirent **namelist;
    char file_long_name[1024];

    int idx;
    int dev_n;
    int tmp_fd;
    int retcode = 0;
    int kbd_fd_count = 0;

    int led_caps_present;
    int key_caps_present;

    /*
     * Will think that device is a keyboard if 'CapsLock' key and 'CapsLock' LED are present!!!
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
        }

        led_caps_present = has_device_LED(tmp_fd, Caps_inst->Led_val, Caps_inst->Led_str);
        //key_caps_present = has_device_KEY(tmp_fd, Caps_inst->Key_val, Caps_inst->Key_str);
        key_caps_present = 1;

        if (led_caps_present == 1 && key_caps_present == 1) {
            kbd_fd_arr[kbd_fd_count] = tmp_fd;
            kbd_fd_count++;
            continue;
        }

        close(tmp_fd);
    }

    for (idx = 0; idx < dev_n; idx++)
        free(namelist[idx]);
    free(namelist);

    return kbd_fd_count;
}


int show_kbd_curr_state(int kbd_fd, struct VirtLed *Caps_inst,
        struct VirtLed *Num_inst, struct VirtLed *Scroll_inst) {

    char kbd_name[256]= "Unknown type";
    int curr_state;

    static char absent_msg[] = "---";
    char* curr_caps_state   = absent_msg;
    char* curr_num_state    = absent_msg;
    char* curr_scroll_state = absent_msg;

    if(ioctl(kbd_fd, EVIOCGNAME(sizeof(kbd_name)), kbd_name) < 0) {
        log_fatal("ioctl(EVIOCGNAME)");
    }

    if ( has_device_LED(kbd_fd, Caps_inst->Led_val, Caps_inst->Led_str) ) {
        curr_state = get_current_LED_state (kbd_fd, Caps_inst->Led_val, Caps_inst->Led_str);
        curr_caps_state = LED_state[curr_state];
    }

    if ( has_device_LED(kbd_fd, Num_inst->Led_val, Num_inst->Led_str) ) {
        curr_state = get_current_LED_state (kbd_fd, Num_inst->Led_val, Num_inst->Led_str);
        curr_num_state = LED_state[curr_state];
    }

    if ( has_device_LED(kbd_fd, Scroll_inst->Led_val, Scroll_inst->Led_str) ) {
        curr_state = get_current_LED_state (kbd_fd, Scroll_inst->Led_val, Scroll_inst->Led_str);
        curr_scroll_state = LED_state[curr_state];
    }

    log_info("Keyboard: '%s' ---> CapsLock(%s)   NumLock(%s)   ScrollLock(%s)", kbd_name,
             curr_caps_state, curr_num_state, curr_scroll_state);

    return 0;
}


//===============//===============//===============//===============//
int main(int argc, char **argv) {

    log_set_level(LOG_FATAL);
    int show_kbd_info = -1;

    struct VirtLed Caps = {.inWork = -1,
            .Led_val = LED_CAPSL, .Led_str = "CapsLock",
            .Key_val = KEY_CAPSLOCK, .Key_str = "CapsLock",
            .currState = -1, .newState = -1};

    struct VirtLed Num = {.inWork = -1,
            .Led_val = LED_NUML, .Led_str = "NumLock",
            .Key_val = KEY_NUMLOCK, .Key_str = "NumLock",
            .currState = -1, .newState = -1};

    struct VirtLed Scroll = {.inWork = -1,
            .Led_val = LED_SCROLLL, .Led_str = "ScrollLock",
            .Key_val = KEY_SCROLLLOCK, .Key_str = "ScrollLock",
            .currState = -1, .newState = -1};

    int kbd_fd_arr[KBD_FD_MAX] = {-1};

    //int retcode = 0;
    int kbd_in_system = 0;

    //int idx;

    // Parsing input arguments
    pars_args(argc, argv, &show_kbd_info, &Caps, &Num, &Scroll);

    if (Caps.inWork == 1)
        log_info("Set %s LED to: %s", Caps.Led_str, LED_state[Caps.newState]);
    if (Num.inWork == 1)
        log_info("Set %s LED to: %s", Num.Led_str, LED_state[Num.newState]);
    if (Scroll.inWork == 1)
        log_info("Set %s LED to: %s", Scroll.Led_str, LED_state[Scroll.newState]);
    if (show_kbd_info == 1)
        log_info("Show all keyboards LED's state", show_kbd_info);
    //------------------------------------------


    kbd_in_system = find_all_keyboards(kbd_fd_arr, &Caps);
    if (kbd_in_system == 0 ) {
        log_fatal("There is no keyboard attached to the computer!!!");
        return 0;
    }
    log_trace("Found '%d' keyboard(s) in the system", kbd_in_system);
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
    for (int idx=0; idx < kbd_in_system; idx++) {
        show_kbd_curr_state(kbd_fd_arr[idx], &Caps, &Num, &Scroll);

        set_LED(kbd_fd_arr[idx], &Caps);
        set_LED(kbd_fd_arr[idx], &Num);
        set_LED(kbd_fd_arr[idx], &Scroll);

        show_kbd_curr_state(kbd_fd_arr[idx], &Caps, &Num, &Scroll);

        close (kbd_fd_arr[idx]);
    }

    return 0;
}

