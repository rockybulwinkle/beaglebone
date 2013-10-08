#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gpio_map.h"

int main(){
	int fd = open("/dev/mem", O_RDWR);
	gpio_control * gpio_0 = map_gpio(fd, GPIO0_BASE);
	gpio_control * gpio_1 = map_gpio(fd, GPIO1_BASE);

	*(gpio_1->oe) &= ~P9_12;
	*(gpio_0->oe) &= ~P9_11;
	*(gpio_0->oe) |= P9_21;
	*(gpio_0->oe) |= P9_22;
	while(1){
		if ((*(gpio_0->in)) & P9_22){
			*(gpio_1->out) |= P9_12;
		} else{
			*(gpio_1->out) &= ~P9_12;
		}
		if ((*(gpio_0->in)) & P9_21){
			*(gpio_0->out) |= P9_11;
		} else{
			*(gpio_0->out) &= ~P9_11;
		}
	}
	return 0;
}


