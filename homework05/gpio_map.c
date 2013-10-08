#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gpio_map.h"

gpio_control *  map_gpio(int fd, unsigned int base){
	volatile void * gpio_addr = mmap(0, GPIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, base); //_must_ be a volatile void *
	gpio_control * gpio = malloc(sizeof(gpio_control));
	gpio->base = gpio_addr;
	gpio->set = gpio->base + GPIO_SET;
	gpio->clear = gpio->base + GPIO_CLEAR;
	gpio->out = gpio->base + GPIO_OUT;
	gpio->in = gpio->base + GPIO_IN; 
	gpio->oe = gpio->base + GPIO_OE;
	return gpio;
}

