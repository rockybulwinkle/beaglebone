// From : http://stackoverflow.com/questions/13124271/driving-beaglebone-gpio-through-dev-mem
//
// Be sure to set -O3 when compiling.
// Modified by Mark A. Yoder  26-Sept-2013
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <signal.h>    // Defines signal-handling functions (i.e. trap Ctrl-C)

#define GPIO0_BASE	0x44E07000
#define GPIO_OE		0x134
#define GPIO_OUT	0x13C
#define GPIO_IN		0x138
#define GPIO_SIZE	0x2000
#define GPIO_SET	0x194
#define GPIO_CLEAR	0x190
#define P9_11		0b01000000000000000000000000000000


/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;    // Set to 0 when ctrl-c is pressed

/****************************************************************
 * signal_handler
 ****************************************************************/
void signal_handler(int sig);
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf( "\nCtrl-C pressed, cleaning up and exiting..\n" );
	keepgoing = 0;
}

int main(int argc, char *argv[]) {
    volatile void *gpio_addr;
    volatile unsigned int *gpio_oe_addr;
    volatile unsigned int *gpio_setdataout_addr;
    volatile unsigned int *gpio_cleardataout_addr;
    unsigned int reg;
    
    // Set the signal callback for Ctrl-C
    signal(SIGINT, signal_handler);

    int fd = open("/dev/mem", O_RDWR);


    gpio_addr = mmap(0, GPIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO0_BASE);

    gpio_oe_addr           = gpio_addr + GPIO_OE;
    gpio_setdataout_addr   = gpio_addr + GPIO_SET;
    gpio_cleardataout_addr = gpio_addr + GPIO_CLEAR;

    if(gpio_addr == MAP_FAILED) {
        printf("Unable to map GPIO\n");
        exit(1);
    }

    // Set USR3 to be an output pin
    //reg = *gpio_oe_addr;
    //reg &= ~P9_11;       // Set USR3 bit to 0
    //*gpio_oe_addr = reg;
    *gpio_oe_addr &= ~P9_11;
    
    while(keepgoing) {
        // printf("ON\n");
        *gpio_setdataout_addr = P9_11;
        usleep(250000);
        // printf("OFF\n");
        *gpio_cleardataout_addr = P9_11;
        usleep(250000);
    }

    munmap((void *)gpio_addr, GPIO_SIZE);
    close(fd);
    return 0;
}
