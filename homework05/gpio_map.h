#ifndef MY_GPIO_MAPPING
#define MY_GPIO_MAPPING

#define GPIO0_BASE	0x44E07000
#define GPIO1_BASE	0x4804C000

#define GPIO_OE		0x134
#define GPIO_OUT	0x13C
#define GPIO_IN		0x138
#define GPIO_SIZE	0x2000
#define GPIO_SET	0x194
#define GPIO_CLEAR	0x190
/*
 * on GPIO 0
 */
#define P9_11		1<<30
#define P9_21		1<<3

/*
 * on GPIO 1
 */
#define P9_12		1<<28

typedef struct {
	volatile unsigned int * set;
	volatile unsigned int * clear;
	volatile unsigned int * out;
	volatile unsigned int * in;
	volatile unsigned int * oe;
	volatile void * base;
}gpio_control;

gpio_control *  map_gpio(int fd, unsigned int base);

#endif
