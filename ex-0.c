#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/kd.h>  /* Keyboard IOCTLs */
#include <sys/ioctl.h> /* ioctl()         */
#include <sys/types.h>
 
    #define ERROR -1
    
    long int arg; /* Where the LED states will be put into.       */

    void checkleds();

	void main()
        {
          int fd;       /* Console fd (/dev/tty). Used as fd in ioctl() */
          

          /* To use as the fd in ioctl(). */
          if ((fd = open("/dev/pts/3", O_NOCTTY)) == ERROR) {
	     perror("open");
	     exit(ERROR);
          }

          /* Value stored in arg. */
	  if (ioctl(fd, KDGETLED, &arg) == ERROR) {
	     perror("ioctl");
	     close(fd);
	     exit(ERROR);
          }

          /* Here we print out current LEDS. */
          checkleds();
        }

        void checkleds()
        {
          /* LED_SCR = 0x1, LED_NUM = 0x2, LED_CAP = 0x4 */

          if (arg == LED_SCR) printf("Scroll Lock LED is on.\n");
	  else if (arg == LED_NUM) printf("Numeric Lock LED is on.\n");
	  else if (arg == LED_CAP) printf("Caps Lock LED is on.\n");

          else if (arg == LED_NUM + LED_CAP) 
             printf("Numeric Lock and Caps Lock LEDs are on.\n");

          else if (arg == LED_NUM + LED_SCR) 
             printf("Numeric Lock and Scroll Lock LEDs are on.\n");

	  else if (arg == LED_CAP + LED_SCR) 
	     printf("Caps Lock and Scroll Lock LEDs are on.\n");

          else if (arg == LED_NUM + LED_SCR + LED_CAP) 
             printf("Numeric Lock, Scroll Lock, and Caps Lock LEDs are on.\n");

        }

